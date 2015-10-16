#pragma once

using uint32 = unsigned int;
using uint64 = unsigned long long;
using uint16 = unsigned short;

#define ServerPort			9000
#define IMPOSSIBLE_RETURN	(uint32)-1
#define ONE_HOUR			60*60

#define PacketHeadLen		sizeof(int32_t)
#define PacketMsgIdMask		0x7fff0000
#define PacketMsgLenMask	0x0000ffff
#define Max_DataBufferSize	64 * 1024 - 1
#define Max_DataSize		Max_DataBufferSize - PacketHeadLen
#define Max_UserDataSize	Max_DataSize - 128