This program records webcam videos together with the local PC time recorded for each frame. It thus allows to tag each frame with the local PC time.
The PC local time is:
- estimated at best (but there is not possible guaranty of an exact time for webcams);
- outputed on stdout for each frame (redirect to save in a file)

An output is given for each gstreamer state change and new frame. It is in the following format:
STATE,HOUR:MINUTE:SECOND.MILLISECONDS,UNIXTIME

STATE values can be NULL, READY, PAUSE, PLAYING, FRAME (see gstreamer states)
UNIXTIME is expressed in seconds with milliseconds after the dot.

Dependancies:
- gstreamer 1.0 (tested with 1.6.1)
- Qt 5 (tested with 5.5.1)
