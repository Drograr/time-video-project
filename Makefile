timedVideo: TimedVideo.cpp
	gcc `pkg-config --cflags --libs gstreamer-1.0` TimedVideo.cpp -o timedVideo

clean:
	rm timedVideo
