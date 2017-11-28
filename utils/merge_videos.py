import pandas as pd
import numpy as np
import os
import logging

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

logging.basicConfig(level=logging.INFO)

# This scripts takes two videos and merge then
# This is done according to a start / end times for synchronysation
video1_filename = '/home/chanel/data/EATMINT-2-bis/ExpeBackup/D02/D02_S01_video.mp4'
time1_filename = '/home/chanel/data/EATMINT-2-bis/ExpeBackup/D02/D02_S01_video_times.csv'
video2_filename = '/home/chanel/data/EATMINT-2-bis/ExpeBackup/D02/D02_S02_video.mp4'
time2_filename = '/home/chanel/data/EATMINT-2-bis/ExpeBackup/D02/D02_S02_video_times.csv'
out_filename = '/tmp/test.mkv'
start_time = 1453480547
end_time = 1453482347


# Now segment the videos
cut_video(video1_filename, time1_filename, start_time, end_time, '/tmp/1.mkv')
cut_video(video2_filename, time2_filename, start_time, end_time, '/tmp/2.mkv')

# Merge the videos
os.system('ffmpeg -y -i /tmp/1.mkv -i /tmp/2.mkv -map 0:0 -filter_complex "[0:1][1:1] amix" -c:v libx264 -preset ultrafast {}'.format(out_filename))
