import pandas as pd
import numpy as np
import os
import logging
import argparse
import tempfile

def cut_video(video_filename, time_filename, start_time, end_time, out_filename, precise=True):
    # Load time file and select frames events only
    time = pd.read_csv(time_filename, header=None, names=['Event', 'Time','UnixTime'])
    time = time[time['Event'] == 'FRAME']

    # Find first frame index (i.e. just before start_time) and number frames
    # (to reach just after end_time)
    idx = np.logical_and(time['UnixTime'] > start_time, time['UnixTime'] < end_time)
    startFrame = np.argmax(idx) - 1
    # nbFrames = idx.sum() + 2
    # os.system('x264 --bframes 0 --seek {startFrame} --frames {nbFrames} -o {out} {input}'.format(startFrame=startFrame, nbFrames=nbFrames, out=out_filename, input=video_filename))
    nbFrames = idx.sum() + 1
    startVideoTime = time.iloc[startFrame]['UnixTime'] - time.iloc[0]['UnixTime']
    endVideoTime = time.iloc[startFrame + nbFrames]['UnixTime'] - time.iloc[0]['UnixTime']

    logging.info("file: {} - startFrame: {} - nbFrames: {} - startTime: {} - endTime {}".format(video_filename, startFrame, nbFrames, startVideoTime, endVideoTime))
    if precise:
        os.system('ffmpeg -y -i {input} -ss {start}  -to {stop} -acodec copy -vcodec libx264 -fflags +genpts {out}'.format(start=startVideoTime, stop=endVideoTime, out=out_filename, input=video_filename))
    else:
        os.system('ffmpeg -y -i {input} -ss {start}  -to {stop} -c copy -fflags +genpts {out}'.format(start=startVideoTime, stop=endVideoTime, out=out_filename, input=video_filename))

def main():
    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser(description='Cut and merge two videos according to start / end times')
    parser.add_argument('start', type=float, help='start time of the merged video')
    parser.add_argument('stop', type=float, help='stop time of the merged video')
    parser.add_argument('out_file', type=str, help='merged video filename')
    parser.add_argument('-v', '--videos', type=str, nargs='+', help='video files to merge together, all sounds will be merged, only the video of the first is kept')
    parser.add_argument('-t', '--times', type=str, nargs='+', help='time files which should match the order of video files')
    parser.add_argument('-p','--precise', action='store_true', help='activate precise mode by reenconding cuted videos to x264')
    args = parser.parse_args()

    N = len(args.videos)

    # Define tempory cutted files names
    cut_files = [os.path.join(tempfile.gettempdir(), '{}.mkv'.format(i)) for i in range(N)]

    # Now segment the videos
    for i in range(N):
        cut_video(args.videos[i], args.times[i], args.start, args.stop, cut_files[i], precise)

    # Merge the videos
    ffmpeg_inputs = ' '.join(['-i {}'.format(f) for f in cut_files])
    os.system('ffmpeg -y ' + ffmpeg_inputs + ' -map 0:0 -filter_complex "[0:1][1:1] amix" -c:v libx264 -preset ultrafast {}'.format(args.out_file))

    # Clean tempdir
    for f in cut_files:
        os.remove(f)


if __name__ == '__main__':
    main()
