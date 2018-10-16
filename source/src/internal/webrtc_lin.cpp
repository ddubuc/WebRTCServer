#include <rtc_base/strings/string_builder.h>
#include <api/units/data_size.h>

//#include "internal/opencvrenderer.h"

namespace webrtc
{

	std::string ToString(const DataSize& value) {
		char buf[64];
  
		rtc::SimpleStringBuilder sb(buf);
		if (value.IsInfinite()) {
			sb << "inf bytes";
		} else {
			sb << value.bytes() << " bytes";
		}
		return sb.str();
	}
}
