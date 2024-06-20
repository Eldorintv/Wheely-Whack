#ifndef WWINCLUDE_H
#define WWINCLUDE_H

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

#include "..\WWClient\UDP6send.h"
#include "..\WWClient\UDP6receive.h"

// std::min did not work due to some macro overwriting (from UDPsend6.h), so undefining min max here 
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif

