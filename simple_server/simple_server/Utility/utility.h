#pragma once

#ifndef ConstructMsgMacro
#define ConstructMsgMacro(msgid, msg) \
	char msg##__[PlayerConnection::Max_UserDataSize + 2] = {0}; \
	assert(msg.SerializePartialToArray(msg##__, sizeof(msg##__))); \
	int msg##_size__ = (msgid << 16) | msg.ByteSize();
#endif
