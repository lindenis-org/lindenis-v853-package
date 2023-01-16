#ifndef __SUNXI_CAM_H__
#define __SUNXI_CAM_H__

#include <string>
#include <sstream>
#include "camerav4l2.h"

namespace sunxi_cam {

struct ImageFrame{
  int64_t image_id;
  int64_t lost_image_num;
  int64_t exp_time;
  int64_t image_timestamp;
  uint8_t *data;
};

class SunxiCam {
	public:
		SunxiCam();
		~SunxiCam();

		bool OpenCamera(const int video_index,
					const int format,
					int fps,
					int width, int height);
		bool CloseCamera(void);

		bool setExposureGain(int exp_val, int gain_val);
		bool getExposureGain(int *exposure, int *gain);
		bool setAutoExposureGain(void);

		bool WaitingBuf(void);

		bool GetImageFrame(ImageFrame *image_frame);
		bool ReturnImageFrame(ImageFrame *image_frame);
		int  GetCameraFd() {
			return camera ? camera->videofd : -1;
		}

		camera_hal *camera;
	private:

		/* camera init */
		int start(const int video_index,
					const int format,
					int fps,
					int width, int height);

		/* shutdown camera */
		int shutdown(void);

		typedef enum{
			CAM_CLOSE, CAM_OPEN, CAM_REQUESTBUF, CAM_STREAMON,
		} cam_state;

		cam_state state;
};

}

#endif
