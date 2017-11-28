import pandas as pd
import numpy as np
import os
import logging
import argparse
import tempfile

def cut_video(video_filename, time_filename, start_time, end_time, out_filename):
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
    os.system('ffmpeg -y -i {input} -ss {start}  -to {stop} -c copy -fflags +genpts {out}'.format(start=startVideoTime, stop=endVideoTime, out=out_filename, input=video_filename))


def main():
    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser(description='Cut and merge two videos according to start / end times')
    parser.add_argument('video_files', type=str, nargs=2, help='video files to merge together, sound will be merged from the two only the video of the first is kept')
    parser.add_argument('time_files', type=str, nargs=2, help='time files correponding to video_files')
    parser.add_argument('start', type=float, help='start time of the merged video')
    parser.add_argument('stop', type=float, help='stop time of the merged video')
    args = parser.parse_args()

    # Define tempory cutted files names
    tmp_file = os.path.join(tempfile.gettempdir(), '{}.mkv')

    # Now segment the videos
    for i, v in enumerate(args.video_files):

        cut_video(v, args.time_files[i], args.start, args.stop, tmp_file.format(i))

    # Merge the videos
    os.system('ffmpeg -y -i /tmp/1.mkv -i /tmp/2.mkv -map 0:0 -filter_complex "[0:1][1:1] amix" -c:v libx264 -preset ultrafast {}'.format(out_filename))

    # Clean tempdir
    for i, v in enumerate(args.video_files):
        os.remove(tmp_file.format(i))


if __name__ == '__main__':
    main()
