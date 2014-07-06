/*
 * Copyright (c) 2014 Andrey Karpenko <andrey@delfa.net>.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "macbind.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include <libaudcore/audstrings.h>
#include <audacious/debug.h>
#include <audacious/i18n.h>
#include <audacious/plugin.h>
#include <audacious/audtag.h>

typedef struct {
	VFSFile *fd;
	void *pAPEDecompress;

	int64_t seek;
	bool_t stop;
	bool_t stream;
	Tuple *tu;

//	gchar *title;

	// media information
	long samplerate;
	int channels;
	unsigned int bits_per_sample;
	unsigned int length_in_ms;
	unsigned int block_align;
	int sample_format;

//	gint seek_to;

	GThread *decoder_thread;

} APEPlaybackContext;


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int64_t setup_read (void * ptr, int64_t size, int64_t nmemb, void* file)
{
	return vfs_fread (ptr, size, nmemb, (VFSFile *)file);
}

static int setup_lseek (void* file, int64_t offset, int whence)
{
	return ( vfs_fseek ( (VFSFile*) file, offset, whence));
}

static void* setup_open(const char * path, const char * mode)
{
	return (void*)vfs_fopen (path, mode);
}

static int setup_close(void* file)
{
	return	vfs_fclose ( (VFSFile*) file);
}

static int64_t setup_ftell(void* file)
{
	return vfs_ftell((VFSFile*)file);
}

static bool_t aud_ape_init (void)
{
	AUDDBG("initializing vfscio class\n");

	SetUpVfsInterface(setup_read, setup_lseek, setup_open, setup_close, setup_ftell);

	return TRUE;
}

static void aud_ape_deinit(void)
{
	AUDDBG("deinitializing mac library\n");
}

static bool_t ape_probe_for_fd (const char * fname, VFSFile * file)
{
	if (! file)
		return FALSE;
	SetUpVfsInterface(setup_read, setup_lseek, setup_open, setup_close, setup_ftell);
	void* APED;
	int error;
	AUDDBG("Trying to create APED\n");
	APED = createAPEDvfs(file, &error);
	if(APED == NULL)
	{
		AUDDBG("APED == NULL error %d\n",error);
		return FALSE;
	}else
	{
		deleteAPED(APED);
	}

	return TRUE;
}

static Tuple * ape_probe_for_tuple (const char * filename, VFSFile * file)
{
	if (! file)
		return NULL;

	bool_t stream = vfs_is_streaming (file);

	SetUpVfsInterface(setup_read, setup_lseek, setup_open, setup_close, setup_ftell);
	void* APED;
	int error;
//	AUDDBG("Trying to create APED\n");
	APED = createAPEDvfs(file, &error);
	if(APED == NULL)
	{
		AUDDBG("APED == NULL error %d\n",error);
		return NULL;
	}

	Tuple * tuple = tuple_new_from_filename (filename);

	tuple_set_str (tuple, FIELD_CODEC, NULL, "Monkey's Audio");
	tuple_set_str (tuple, FIELD_QUALITY, NULL, "Loss less");
	tuple_set_int (tuple, FIELD_BITRATE, NULL, GetInfoAverageBitRateAPED(APED) );
	tuple_set_int (tuple, FIELD_LENGTH, NULL, GetInfoLengthMsAPED(APED) );


	deleteAPED(APED);

	if (! stream)
	{
		vfs_rewind (file);
		//tag_tuple_read (tuple, file);
	}

	return tuple;

}

static void ape_stop_playback_worker (InputPlayback * data)
{
	pthread_mutex_lock (& mutex);
	APEPlaybackContext * context = data->get_data (data);

	if (context != NULL)
	{
		context->stop = TRUE;
		data->output->abort_write ();
	}

	pthread_mutex_unlock (& mutex);
}


static void ape_pause_playback_worker (InputPlayback * data, bool_t pause)
{
	pthread_mutex_lock (& mutex);
	APEPlaybackContext * context = (APEPlaybackContext *) data->get_data (data);

	if (context != NULL)
		data->output->pause (pause);

	pthread_mutex_unlock (& mutex);
}

static void ape_seek_time (InputPlayback * data, int time)
{
	pthread_mutex_lock (& mutex);
	APEPlaybackContext * context = (APEPlaybackContext *) data->get_data (data);

	if (context != NULL)
	{
		context->seek = time;
		data->output->abort_write ();
	}

	pthread_mutex_unlock (& mutex);
}

static bool_t ape_playback_worker (InputPlayback * data, const char *
 filename, VFSFile * file, int start_time, int stop_time, bool_t pause)
{
	if (! file)
		return FALSE;

	bool_t error = FALSE;
	APEPlaybackContext ctx;
	char* outbuf = NULL;
	int ret;
	int bitrate = 0, bitrate_sum = 0, bitrate_count = 0;
	int bitrate_updated = -1000; /* >= a second away from any position */

	int error_count = 0;

	memset(&ctx, 0, sizeof(APEPlaybackContext));

	AUDDBG("playback worker started for %s\n", filename);
	ctx.fd = file;

	AUDDBG ("Checking for streaming ...\n");
	ctx.stream = vfs_is_streaming (file);
	ctx.tu = ctx.stream ? get_stream_tuple (data, filename, file) : NULL;

	ctx.seek = (start_time > 0) ? start_time : -1;
	ctx.stop = FALSE;
	data->set_data (data, & ctx);


	SetUpVfsInterface(setup_read, setup_lseek, setup_open, setup_close, setup_ftell);
	ctx.pAPEDecompress = createAPEDvfs(file, &ret);

	if (ctx.pAPEDecompress == 0)
	{
OPEN_ERROR:
		fprintf (stderr, "APE: Error opening %s: %d.\n", filename, ret);
		error = TRUE;
		goto cleanup;
	}
	ctx.block_align = GetInfoBlockAlignAPED(ctx.pAPEDecompress);
	outbuf = malloc (1024 * ctx.block_align);
	size_t outbuf_size = 0;

GET_FORMAT:

	ctx.samplerate = GetInfoSampleRateAPED(ctx.pAPEDecompress);
	ctx.channels = GetInfoChannelsAPED(ctx.pAPEDecompress);
	ctx.bits_per_sample = GetInfoBitsPerSampleAPED(ctx.pAPEDecompress);

	AUDDBG("Block align %d Sample Rate %d Channels %d Bits Per Sample %d\n",(int)ctx.block_align, (int)ctx.samplerate,\
			(int)ctx.channels, (int)ctx.bits_per_sample);

	int blocksread;
	if( GetDataAPED(ctx.pAPEDecompress, outbuf, 1024, &blocksread ) != 0)
		goto OPEN_ERROR;


	bitrate = ctx.channels * ctx.bits_per_sample * ctx.samplerate;
	data->set_params (data, bitrate, ctx.samplerate, ctx.channels);

	switch(ctx.block_align/ctx.channels)
	{
	case 1:
		ctx.sample_format = FMT_S8;
		break;
	case 2:
		ctx.sample_format = FMT_S16_LE;
		break;
	case 4:
		ctx.sample_format = FMT_S32_LE;
		break;
	default:
		AUDDBG("Can't select Sample Format\n");
		goto cleanup;
	}

	if (! data->output->open_audio (ctx.sample_format, ctx.samplerate, ctx.channels))
	{
		error = TRUE;
		goto cleanup;
	}

	data->output->flush (start_time);

	if (pause)
		data->output->pause (TRUE);

	data->set_gain_from_playlist (data);

	pthread_mutex_lock (& mutex);

	AUDDBG("starting decode\n");
	data->set_pb_ready(data);

	pthread_mutex_unlock (& mutex);

	int64_t frames_played = 0;
	int64_t frames_total = (int64_t) (stop_time - start_time) * ctx.samplerate / 1000;

	while (1)
	{
		pthread_mutex_lock (& mutex);

		if (ctx.stop)
		{
			pthread_mutex_unlock (& mutex);
			break;
		}

		if (ctx.seek >= 0)
		{
			if (SeekAPED(ctx.pAPEDecompress, (int64_t) ctx.seek * ctx.samplerate / 1000) != 0)
			{
				fprintf (stderr, "APE error in %s\n", filename);
			}
			else
			{
				data->output->flush (ctx.seek);
				frames_played = (int64_t) (ctx.seek - start_time) * ctx.samplerate / 1000;
				outbuf_size = 0;
			}

            ctx.seek = -1;
		}

		pthread_mutex_unlock (& mutex);

/*		mpg123_info(ctx.decoder, &fi);
		bitrate_sum += fi.bitrate;
		bitrate_count ++;

		if (bitrate_sum / bitrate_count != bitrate && abs
		 (data->output->written_time () - bitrate_updated) >= 1000)
		{
			data->set_params (data, bitrate_sum / bitrate_count * 1000,
			 ctx.rate, ctx.channels);
			bitrate = bitrate_sum / bitrate_count;
			bitrate_sum = 0;
			bitrate_count = 0;
			bitrate_updated = data->output->written_time ();
		}*/

		if (ctx.stream)
			update_stream_tuple (data, file, ctx.tu);

		ret = GetDataAPED(ctx.pAPEDecompress, outbuf, 1024, &blocksread);
		if ( blocksread == 0 || ret  != 0 )
		{
			if (ret == 0 )
				goto decode_cleanup;

			fprintf (stderr, "APE error in %s\n", filename);

			if (++ error_count >= 10)
			{
				error = TRUE;
				goto decode_cleanup;
			}
		}
		else
		{
			error_count = 0;

			bool_t stop = FALSE;

			if (stop_time >= 0)
			{
				int64_t remain = frames_total - frames_played;
				remain = MAX (0, remain);

				if (blocksread >= remain)
				{
					blocksread = remain;
					stop = TRUE;
				}
			}

			data->output->write_audio (outbuf, blocksread*ctx.block_align);
			frames_played += blocksread;
			blocksread = 0;

			if (stop)
				goto decode_cleanup;
		}
	}

decode_cleanup:
	AUDDBG("decode complete\n");
	pthread_mutex_lock (& mutex);
	data->set_data (data, NULL);
	pthread_mutex_unlock (& mutex);

cleanup:
	deleteAPED(ctx.pAPEDecompress);
	free(outbuf);
	if (ctx.tu)
		tuple_unref (ctx.tu);
	return ! error;
}

static const char ape_about[] =
 N_("Original code by\n"
    "Andrey Karpenko <andrey@delfa.net>\n\n"
    "http://www.borinstruments.delfa.net/");

/** plugin description header **/
static const char *ape_fmts[] = { "mac", "ape", "apl", NULL };

AUD_INPUT_PLUGIN
(
	.name = N_("APE Plugin"),
	.domain = PACKAGE,
	.about_text = ape_about,
	.init = aud_ape_init,
	.cleanup = aud_ape_deinit,
	.extensions = ape_fmts,
	.is_our_file_from_vfs = ape_probe_for_fd,
	.probe_for_tuple = ape_probe_for_tuple,
	.play = ape_playback_worker,
	.stop = ape_stop_playback_worker,
	.mseek = ape_seek_time,
	.pause = ape_pause_playback_worker,
	.update_song_tuple = NULL, //ape_write_tag,
	.get_song_image = NULL, //ape_get_image,
)
