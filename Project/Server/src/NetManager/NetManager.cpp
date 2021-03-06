/*
 * NetManager.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: xuke
 */

#include "NetManager.h"

namespace basic {

int NetManager::Init()
{
	if(InitNet()==-1)
	{
		CloseNet();
		return -1;
	}
	libev_NetManager_Init();
	return 0;
}

int NetManager::InitNet()
{
	const char* ipStr="192.168.1.109";
	const int port=7878;
	const int maxConnectCout=0;

	Server=socket_class();
	//memset(&Server,0,sizeof(Server));
	//生成SOCKet描数字
	if((Server._fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	{
		LogManager::LogError("Init Server_fd Error:");
		return -1;
	}
	int reuse=1;
    if (setsockopt(Server._fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))==-1)
    {
            LogManager::LogError("setsockopet error:");
            return -1;
    }
	//Socket结构封装
	Server._addr.sin_addr.s_addr=inet_addr(ipStr);
   // Server._addr.sin_addr.s_addr=INADDR_ANY;
	Server._addr.sin_family=AF_INET;
	Server._addr.sin_port=htons(port);
	printServerinfo(Server);
	//把socket描数字，socket地址结构绑定
	if(bind(Server._fd,(const struct sockaddr *)&Server._addr,sizeof(Server._addr))==-1)
	{
		LogManager::LogError("Bind error:");
		return -1;
	}
	if(listen(Server._fd,maxConnectCout)==-1)
	{
		LogManager::LogError("Listen Error:");
		return -1;
	}
	xk_Debug::Log()<<"Net Init Finish"<<endl;
	return 0;
}

int NetManager::NetAcceptClient_Block(socket_class& client)
{
		socklen_t len;
		if((client._fd=accept(Server._fd,(struct sockaddr*)&(client._addr),&len))==-1)
		{
			LogManager::LogError("Accept Error:");
			return -1;
		}
		return 0;
}

int NetManager::NetAcceptClient_NoBlock(socket_class& client)
{
		socklen_t len;
		if((client._fd=accept(Server._fd,(struct sockaddr*)&(client._addr),&len),SOCK_NONBLOCK|SOCK_CLOEXEC)==-1)
		{
			LogManager::LogError("Accept Error:");
			return -1;
		}
		return 0;	
}

//Success:0,Error:-1
int NetManager::CloseNet()
{
	//shutdown(server_fd,SHUT_WR);
	if(close(Server._fd)==-1)
	{
		LogManager::LogError("Close Server Error:");
		return -1;
	}
	xk_Debug::Log()<<"Server Closed"<<endl;
	return 0;
}
struct socket_class NetManager::getServerSocketInfo()
{
	return Server;
}

int NetManager::printServerinfo(const socket_class& _socket)
{
	cout<<"Server:  fileId ："<<_socket._fd<<"  IP:"<<inet_ntoa(_socket._addr.sin_addr)<<"    Port："<<(ntohs(_socket._addr.sin_port))<<endl;
}

ClientInfoPool::ClientInfoPool(socket_class* socket_r)
{
	mClientInfo=new ClientInfo();
	mClientInfo->mSocketInfo=socket_r;
	mNetPackageReceivePool=new NetPackageReceivePool();
	//run();
}

ClientInfoPool::~ClientInfoPool() {
	delete mClientInfo;
	delete mNetPackageReceivePool;
}
int ClientInfoPool::run()
{
	//NetReceiveMsg_Block();
	if(NetReceiveMsg_NoBlock()==-1)
	{
		ClientManagerPool::getSingle()->RemoveClient(this);
		delete this;
		return -1;
	}
	return 0;
}

int ClientInfoPool::NetReceiveMsg_Block()
{
	const socket_class& client=*(mClientInfo->mSocketInfo);
	while(3)
	{
		unsigned char msg[1024]={};
		ssize_t receiveMsgLength=recv(client._fd,&msg,1024,0);
		if(receiveMsgLength==-1)
		{
			LogManager::LogError("NetReceiveMsg Error:");
			CloseNet();
			printClientinfo();
			return -1;
		}else if(receiveMsgLength==0)
		{
			CloseNet();
			xk_Debug::Log()<<"client DisConnected： "<<endl;
			printClientinfo();
			return 0;
		}else if(receiveMsgLength>0)
		{
			xk_Debug::Log()<<"收到密文字节流，"<<" 长度："<<receiveMsgLength<<endl;
			for(int i=0;i<receiveMsgLength;i++)
			{
				xk_Debug::Log()<<(int)msg[i]<<" | ";
			}
			xk_Debug::Log()<<endl;

			PackagePoolParseData(msg,receiveMsgLength);
		}
	}
	return 0;
}
int ClientInfoPool::NetReceiveMsg_NoBlock()
{
	const int _fd=mClientInfo->mSocketInfo->_fd;
	unsigned char msg[1024]={};
	//ssize_t receiveMsgLength=recv(_fd,&msg,1024,0);
	ssize_t receiveMsgLength=recv(_fd,&msg,1024,MSG_DONTWAIT);
	if(EAGAIN == errno)
	{
		cout<<"What are you  Block Opearte?"<<endl;
	}
	if(receiveMsgLength==-1 && EAGAIN!=errno)
	{
			xk_Debug::LogSystemError("NetReceiveMsg Error:");
			CloseNet();
			printClientinfo();
			return -1;
	}else if(receiveMsgLength==0)
	{
			CloseNet();
			xk_Debug::Log()<<"client DisConnected： "<<endl;
			printClientinfo();
			return -1;
	}else if(receiveMsgLength>0)
	{
			xk_Debug::Log()<<"Receive miwen Byte Stream,"<<" Length："<<receiveMsgLength<<endl;
			for(int i=0;i<receiveMsgLength;i++)
			{
				xk_Debug::Log()<<(int)msg[i]<<" | ";
			}
			xk_Debug::Log()<<endl;
			PackagePoolParseData(msg,receiveMsgLength);
	}
	return 0;
}
int ClientInfoPool::PackagePoolParseData(const unsigned char* msg,int msg_Length)
{
				if(mNetPackageReceivePool==NULL)
				{
					mNetPackageReceivePool=new NetPackageReceivePool();
				}
				mNetPackageReceivePool->ReceiveNetMsg(msg,msg_Length);
				int PackageCout=0;
				while(true)
				{
					mNetPackageReceivePool->GetPackage();
					if(mNetPackageReceivePool->bodyData!=NULL)
					{
						ParseData(mNetPackageReceivePool->bodyData,mNetPackageReceivePool->bodyLength);
						PackageCout++;
					}else
					{
						break;
					}
				}
				cout<<"Parse Package Cout："<<PackageCout<<endl;
}

int ClientInfoPool::ParseData(const unsigned char* msg,int msg_Length)
{
	NetEventPackage mProtobuf=NetEventPackage();
	mProtobuf.mClient=this;
	mProtobuf.DeSerializeStream(msg,msg_Length);

	NetEventManager::getSingle()->handleNetPackage(mProtobuf);
	return 0;
}

int ClientInfoPool::SendData(int command,google::protobuf::Message* msgClass)
{
	Protobuf mProtobuf=Protobuf(command,msgClass);
	NetOutStream mNetOutStream=mProtobuf.SerializeMsgObj();
	NetPackageSendPool mNetSendPool;
	mNetSendPool.SetData(mNetOutStream.msg,mNetOutStream.msg_Length);
	NetSendMsg_NoBlock(mNetSendPool.encryption_data,mNetSendPool.Length);
	return 0;
}

int ClientInfoPool::NetSendMsg_Block(const unsigned char* msg,const int Length)
{
	static int SendCout=0;
	const socket_class& client=*mClientInfo->mSocketInfo;

	xk_Debug::Log()<<"Send MiWen Length："<<Length<<endl;
	for(int i=0;i<Length;i++)
	{
		cout<<(int)(msg[i])<<" | ";
	}
	xk_Debug::Log()<<endl;

	if(send(client._fd,msg,Length,0)==-1)
	{
		LogManager::LogError("Send Error:");
		return -1;
	}else
	{
		SendCout++;
		cout<<"SendCout： "<<SendCout<<endl;
	}
	return 0;
}

int ClientInfoPool::NetSendMsg_NoBlock(const unsigned char* msg,const int Length)
{
	static int SendCout=0;
	const socket_class& client=*mClientInfo->mSocketInfo;

	xk_Debug::Log()<<"Send cipherTextByteStream,Length："<<Length<<endl;
	for(int i=0;i<Length;i++)
	{
		cout<<(int)(msg[i])<<" | ";
	}
	xk_Debug::Log()<<endl;

    if(send(client._fd,msg,Length,MSG_DONTWAIT)==-1)
	{
		LogManager::LogError("Send Error:");
		return -1;
	}else
	{
		SendCout++;
		cout<<"SendCout： "<<SendCout<<endl;
	}
	return 0;
}
int ClientInfoPool::CloseNet()
{
	shutdown(mClientInfo->mSocketInfo->_fd,SHUT_WR);
	if(close(mClientInfo->mSocketInfo->_fd)==-1)
	{
		perror("Client Client：");
	}
}
int ClientInfoPool::printClientinfo()
{
	cout<<"Client:  IoId："<<mClientInfo->mSocketInfo->_fd<<"  IP:"<<inet_ntoa(mClientInfo->mSocketInfo->_addr.sin_addr)<<"  port："<<(ntohs(mClientInfo->mSocketInfo->_addr.sin_port))<<endl;
}

int ClientManagerPool::InitClient(socket_class* info)
{
	ClientInfoPool* mClient=new ClientInfoPool(info);
	AddClient(mClient);
	printClientPoolinfo();
}

int ClientManagerPool::AddClient(ClientInfoPool* client)
{
	mClientList_Mutex.Lock();	
	ClientList.push_back(client);
	mClientList_Mutex.unLock();

	mClientDic_Mutex.Lock();
	ClientDic.insert(pair<int,ClientInfoPool*>(client->mClientInfo->mSocketInfo->_fd,client));
	mClientDic_Mutex.unLock();
	return 0;
}
int ClientManagerPool::RemoveClient(ClientInfoPool* client)
{
	mClientList_Mutex.Lock();
	for(vector<ClientInfoPool*>::iterator iter=ClientList.begin();iter!=ClientList.end();)
	{
		if((*iter)==client)
		{
			ClientList.erase(iter);
			break;
		}else
		{
			iter++;
		}
	}
	mClientList_Mutex.Lock();
	mClientDic_Mutex.Lock();
	for(map<int,ClientInfoPool*>::iterator iter=ClientDic.begin();iter!=ClientDic.end();iter++)
	{
		if((*iter).second==client)
		{
			ClientDic.erase(iter);
			break;
		}
	}
	mClientDic_Mutex.unLock();
	return 0;
}

ClientInfoPool* ClientManagerPool::GetClient(int _fd)
{
	mClientDic_Mutex.Lock();	
	map<int,ClientInfoPool*>::iterator iter=ClientDic.find(_fd);
	if(iter!=ClientDic.end())
	{
		return ((*iter).second);
	}
	mClientDic_Mutex.unLock();
	return 0;
}

vector<ClientInfoPool*>& ClientManagerPool::GetClientPool()
{
	return ClientList;
}

int ClientManagerPool::printClientPoolinfo()
{
	cout<<"ClientPool Info："<<endl;
	mClientList_Mutex.Lock();
	for(vector<ClientInfoPool*>::iterator iter=ClientList.begin();iter!=ClientList.end();iter++)
	{
		(*(*(iter))).printClientinfo();
	}
	mClientList_Mutex.unLock();
	cout<<"Pool capacity："<<ClientList.capacity()<<endl;
	cout<<"Pool size："<<ClientList.size()<<endl;
}
Protobuf::Protobuf()
{
	protobuf_command=-1;
	protobuf_msgObj=0;
	protobuf_Length=-1;
	protobuf_msg=0;
}

Protobuf::Protobuf(int command,google::protobuf::Message* obj)
{
	protobuf_command=command;
	protobuf_msgObj=obj;
	protobuf_Length=obj->ByteSize();
	protobuf_msg=0;
}
void NetPackageReceivePool::ReceiveNetMsg(const unsigned char* msg,const int Length)
{
	int InputStream1Length=Length+InputStreamLength;
	unsigned char* mInputStream1=new unsigned char[InputStream1Length];
	for(int i=0;i<InputStream1Length;i++)
	{
		if(mInputStream!=NULL && InputStreamLength>0)
		{
			if(i<InputStreamLength)
			{
				mInputStream1[i]=mInputStream[i];
			}else
			{
				mInputStream1[i]=msg[i-InputStreamLength];
			}
		}else
		{
			mInputStream1[i]=msg[i];
		}
	}
	delete mInputStream;
	mInputStream=mInputStream1;
	InputStreamLength=InputStream1Length;
}
void NetPackageReceivePool::GetPackage()
{
	SetData(mInputStream,InputStreamLength);
	if(bodyData!=NULL)
	{
			int removeLength=bodyLength+stream_head_Length+msg_head_Length+stream_tail_Length;
		    int InputStream1Length=InputStreamLength-removeLength;
		    if(InputStream1Length<=0)
		    {
		    	delete mInputStream;
		    	mInputStream=NULL;
		    	InputStreamLength=0;
		    	return;
		    }
			unsigned char* mInputStream1=new unsigned char[InputStream1Length];
			for(int i=0;i<InputStream1Length;i++)
			{
				mInputStream1[i]=mInputStream[i+removeLength];
			}
			delete mInputStream;
			mInputStream=mInputStream1;
			InputStreamLength=InputStream1Length;
	}
}
NetPackageReceivePool::NetPackageReceivePool()
{
	mInputStream=NULL;
	InputStreamLength=0;
}
NetPackageReceivePool::~NetPackageReceivePool()
{
	delete mInputStream;
}

void NetPackageSendPool::SendNetMsg(const unsigned char* msg,const int Length)
{
	SetData(msg,Length);

}

void NetEncryptionInputStream::SetData(const unsigned char* msg,const int Length)
{
	if(bodyData!=NULL)
	{
		delete bodyData;
		bodyData=NULL;
		bodyLength=0;
	}
	if(msg==NULL || Length<=0)
	{
		return;
	}else
	{
		cout<<"Pool Parse："<<Length<<endl;
		for(int i=0;i<Length;i++)
		{
			cout<<(int)msg[i]<<" | ";
		}
		cout<<endl;
		cout<<"Pool Parse Finish"<<endl;
	}
	if(Length-stream_head_Length-msg_head_Length-stream_tail_Length<=0)
	{
		cerr<<"Pool Parse Error：sum Length on Enough"<<endl;
		return;
	}
	unsigned char StreamHead[stream_head_Length]={7,7};
	unsigned char StreamTail[stream_tail_Length]={7,7};

	bodyLength=(msg[stream_head_Length+3]<<24 | msg[stream_head_Length+2]<<16 | msg[stream_head_Length+1]<<8 | msg[stream_head_Length]);
	if(bodyLength<=0 || Length-stream_head_Length-msg_head_Length-bodyLength-stream_tail_Length<0)
	{
		cerr<<"Pool Parse Error,package Length no enough："<<bodyLength<<endl;
		return;
	}

	bodyData=new unsigned char[bodyLength];
	for(int i=0;i<bodyLength;i++)
	{
		bodyData[i]=msg[i+msg_head_Length+stream_head_Length];
	}
}
NetEncryptionInputStream::NetEncryptionInputStream()
{
	bodyLength=0;
	bodyData=NULL;
}

NetEncryptionInputStream::~NetEncryptionInputStream()
{
	delete bodyData;
}

void NetEncryptionOutStream::SetData(const unsigned char* msg,const int Length)
{
	this->Length=msg_head_Length+Length+stream_head_Length+stream_tail_Length;
	unsigned char StreamHead[stream_head_Length]={7,7};
	unsigned char StreamTail[stream_tail_Length]={7,7};

	unsigned char lengStr[msg_head_Length]={};
	lengStr[0]=Length;
	lengStr[1]=Length>>8;
	lengStr[2]=Length>>16;
	lengStr[3]=Length>>24;

	encryption_data=new unsigned char[this->Length];
	for(int i=0;i<this->Length;i++)
	{
		if(i<stream_head_Length)
		{
			encryption_data[i]=StreamHead[i];
		}
		else if(i<stream_head_Length+msg_head_Length)
		{
			encryption_data[i]=lengStr[i-stream_head_Length];
		}else if(i<stream_head_Length+msg_head_Length+Length)
		{
			encryption_data[i]=msg[i-msg_head_Length-stream_head_Length];
		}else
		{
			encryption_data[i]=StreamTail[i-msg_head_Length-stream_head_Length-Length];
		}
	}
}

NetEncryptionOutStream::NetEncryptionOutStream()
{
	Length=0;
	encryption_data=NULL;
}

NetEncryptionOutStream::~NetEncryptionOutStream()
{
	delete encryption_data;
}

NetOutStream Protobuf::SerializeMsgObj()
{
	unsigned char* buffer=new unsigned char[protobuf_Length];
	protobuf_msgObj->SerializeToArray(buffer,protobuf_Length);
	NetOutStream mStream=NetOutStream(protobuf_command,protobuf_Length,buffer);
	return mStream;
}

int Protobuf::DeSerializeStream(const unsigned char* msg,int Length)
{
	NetInputStream mStream=NetInputStream(msg,Length);
	protobuf_msg=mStream.buffer;
	protobuf_command=mStream.command;
	protobuf_Length=mStream.buffer_Length;

	return 0;
}

NetInputStream::NetInputStream(const unsigned char* msg1,const int msg1Length)
{
		char* ivStr1=new char[strlen(ivStr)];
		strcpy(ivStr1,ivStr);
		unsigned char* msg= Encryption_AES::getSingle()->Decryption_CBC(msg1,msg1Length,(unsigned char*)keyStr,(unsigned char*)ivStr1);
		int msgLength=Encryption_AES::getSingle()->GetStreamLength(msg,msg1Length);
		delete[] ivStr1;

		cout<<"receive obviousText ByteStream，Length:"<<msgLength<<endl;
		for(int i=0;i<msgLength;i++)
		{
			cout<<(int)msg[i]<<" | ";
		}
		cout<<endl;
		this->command=(msg[3]<<24 | msg[2]<<16 | msg[1]<<8 | msg[0]);
		cout<<"protobuf command ："<<command<<endl;

		buffer_Length=msgLength-command_Length;

		buffer=new unsigned char[buffer_Length];
		for(int i=0;i<buffer_Length;i++)
		{
			buffer[i]=msg[i+command_Length];
		}

		delete[] msg;
}

NetOutStream:: NetOutStream(const int command,const int Length,const unsigned char* data)
{
		unsigned char head[4]={};
		head[0]=command;
		head[1]=command>>8;
		head[2]=command>>16;
		head[3]=command>>24;

		int sumLength=command_Length+Length;
		unsigned char* msgStream=new unsigned char[sumLength];
		for(int i=0;i<sumLength;i++)
		{
			if(i<command_Length)
			{
				msgStream[i]=head[i];
			}else
			{
				msgStream[i]=data[i-command_Length];
			}
		}
		cout<<"Send ObviousText ByteStream，Length:"<<sumLength<<endl;
		for(int i=0;i<sumLength;i++)
		{
			cout<<(int)msgStream[i]<<" | ";
		}
		cout<<endl;

		msg_Length= Encryption_AES::getSingle()->GetCipherLength(sumLength);
		//AES:PKCS7
		unsigned char* msgStream1=new unsigned char[msg_Length];
		for(int i=0;i<msg_Length;i++)
		{
			if(i<sumLength)
			{
				msgStream1[i]=msgStream[i];
			}else
			{
				msgStream1[i]=msg_Length-sumLength;
			}
		}
		delete[] msgStream;
		char* ivStr1=new char[16];
		strcpy(ivStr1,ivStr);
		this->msg= Encryption_AES::getSingle()->Encryption_CBC(msgStream1,msg_Length,(unsigned char*)keyStr,(unsigned char*)ivStr1);
		delete[] ivStr1;
		delete[] msgStream1;
}

} /* namespace basic */

