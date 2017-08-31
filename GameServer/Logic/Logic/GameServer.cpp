#include "GameServer.h"

#include <fstream>
#include <memory>
#include <string>

#include "../../Common/json.hpp"
#include "../../Common/Define.h"
#include "../../Common/ConsoleLogger.h"

#include "../../Network/Network/NetworkMessenger.h"

#include "PacketProcess.h"
#include "UserManager.h"

namespace FPLogic
{
	using json = nlohmann::json;
	using LogType = FPCommon::LogType;

	ErrorCode GameServer::Init()
	{
		// ConsoleLogger 생성.
		_logger = std::make_unique<ConsoleLogger>();

		// 서버 정보 로드.
		if (loadConfig() != ErrorCode::None)
		{
			return ErrorCode::FailConfigLoad;
		}

		// 패킷 큐 생성.
		_recvQueue = std::make_unique<PacketQueue>();
		_sendQueue = std::make_unique<PacketQueue>();

		// 네트워크 메신져 생성.
		_network = std::make_unique<NetworkMessenger>();
		_network->Init(_logger.get(), _config.get(), _recvQueue.get(), _sendQueue.get());
		
		// 유저 관리자 클래스 생성.
		_userManager = std::make_unique<UserManager>();
		_userManager->Init(_config.get()->_maxClientCount);

		// 패킷 처리 클래스 생성
		_packetProcess = std::make_unique<PacketProcess>();
		_packetProcess->Init(_logger.get(), _userManager.get(), _recvQueue.get(), _sendQueue.get());

		return ErrorCode::None;
	}

	void GameServer::Run()
	{
	}

	void GameServer::Stop()
	{
		Release();
	}

	ErrorCode GameServer::loadConfig()
	{
		std::ifstream configFile("C:\\ServerConfig.json");
		if (configFile.fail())
		{
			_logger->Write(LogType::LOG_ERROR, "%s | Config File Load Failed", __FUNCTION__);
			return ErrorCode::FailConfigLoad;
		}

		json configJson;
		configFile >> configJson;

		_config = std::make_unique<ServerConfig>();

		_config->_port = std::stoi(configJson["_port"].get<std::string>());
		_config->_backlog = std::stoi(configJson["_backlog"].get<std::string>());

		_config->_maxClientCount = std::stoi(configJson["_maxClientCount"].get<std::string>());
		_config->_extraClientCount = std::stoi(configJson["_extraClientCount"].get<std::string>());

		_config->_maxClientRecvSize = std::stoi(configJson["_maxClientRecvSize"].get<std::string>());
		_config->_maxClientSendSize = std::stoi(configJson["_maxClientSendSize"].get<std::string>());

		return ErrorCode::None;
	}

	void GameServer::Release()
	{
		if (_network != nullptr)
		{
			_network->Stop();
		}
	}
}