/*
 * NetProtoEvent.h
 *
 *  Created on: Mar 25, 2016
 *      Author: xuke
 */

#ifndef SRC_BASICMODULE_NETMANAGER_NETPROTOEVENT_H_
#define SRC_BASICMODULE_NETMANAGER_NETPROTOEVENT_H_

#include "proto_command.pb.h"
#include "proto_message_one.pb.h"
#include "proto_message_two.pb.h"
#include "proto_struct.pb.h"
#include "xkDebug.h"
#include "NetManager.h"
#include "DbTable.h"
namespace basic
{

class NetEventPackage;
//1100,聊天
int Proto_Rec_Chat(const NetEventPackage mProtobuf);
//1101,注册账号
int Proto_Recevie_RegisterAccount(const NetEventPackage mProtobuf);
//1102,登陆账号
int Proto_Receive_LoginGame(const NetEventPackage mProtobuf);
//1103,选择服务器
int Proto_Receive_SelectServer(const NetEventPackage mProtobuf);
//1104,创建角色
int Proto_Receive_CreateRole(const NetEventPackage mProtobuf);
}

#endif /* SRC_BASICMODULE_NETMANAGER_NETPROTOEVENT_H_ */
