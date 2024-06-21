#ifndef WWINCLUDE_H
#define WWINCLUDE_H

#include "..\WWClient\UDP6send.h"
#include "..\WWClient\UDP6receive.h"

#include "WWhelpers.h"
#include "WWkeybinds.h"
#include "WWmodel.h"
#include "WWtexture.h"

extern "C" {
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"
}

#include <regex>
#include <queue>
#include <mutex>
#include <condition_variable>

#endif
