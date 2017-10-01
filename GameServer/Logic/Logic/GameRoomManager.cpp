#include "GameRoomManager.h"

#include "../../Common/Util.h"

namespace FPLogic
{
	using Util = FPCommon::Util;

	void GameRoomManager::Init(ConsoleLogger * logger, PacketQueue * sendQueue, UserManager * userManager)
	{
		_logger = logger;
		_sendQueue = sendQueue;
		_userManager = userManager;

		_gameRoomPool.Init(maxGameRoomNum);
	}

	// TODO :: ���⼭ �� ���� üũ, ���μ����� �������־�� �Ѵ�.
	void GameRoomManager::UpdateRooms()
	{
		auto countActivateRoom = 0;
		// TODO :: ������Ʈ Ǯ Ŭ������ Range based for Loop�� ������ �� �ֵ���.
		for (auto i = 0; i < _gameRoomPool.GetSize(); ++i)
		{
			// Ȱ��ȭ�� �� ������ ��� ��ü�� �� ���� �ʾƵ� �ǵ���
			if (countActivateRoom >= _gameRoomPool.GetActivatedObjectCount())
			{
				break;
			}

			auto room = &_gameRoomPool[i];

			if (room->GetState() == RoomState::None)
			{
				continue;
			}

			roomProcess(room);
			++countActivateRoom;
		}
	}

	// GameRoom ������Ʈ Ǯ���� �� ���� ã�� ��ȯ�ϴ� �޼ҵ�.
	int GameRoomManager::GetEmptyRoom()
	{
		return _gameRoomPool.GetTag();
	}

	GameRoom * GameRoomManager::GetRoom(const int roomNumber)
	{
		return &_gameRoomPool[roomNumber];
	}

	ErrorCode GameRoomManager::EnterUserToRoom(const int sessionId, const int gameRoomIdx)
	{
		auto& gameRoom = _gameRoomPool[gameRoomIdx];
		auto enteringUser = _userManager->FindUserWithSessionIdx(sessionId);

		return gameRoom.EnterUser(enteringUser);
	}

	ErrorCode GameRoomManager::GameStartAck(const int roomNumber)
	{
		auto room = &_gameRoomPool[roomNumber];

		room->AckGameStart();

		return ErrorCode::None;
	}

	ErrorCode GameRoomManager::TurnChange(const int roomNumber)
	{
		auto room = &_gameRoomPool[roomNumber];

		if (room == nullptr)
		{
			_logger->Write(LogType::LOG_ERROR, "%s | Invalid TurnChange Func call, Room Number(%d)", __FUNCTION__, roomNumber);
			return ErrorCode::InvalidRoomNumber;
		}

		Packet::TurnStartNotify turnStartNotify;
		Packet::EnemyTurnStartNotify enemyTurnStartNotify;

		if (room->_turnPlayer == 1)
		{
			room->_turnPlayer = 2;

			Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_TurnStartNotify, room->_player2->GetSessionIdx(), &turnStartNotify);
			Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_EnemyTurnStartNotify, room->_player1->GetSessionIdx(), &enemyTurnStartNotify);
		}
		else
		{
			room->_turnPlayer = 1;

			Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_TurnStartNotify, room->_player1->GetSessionIdx(), &turnStartNotify);
			Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_EnemyTurnStartNotify, room->_player2->GetSessionIdx(), &enemyTurnStartNotify);
		}

		return ErrorCode::None;
	}

	void GameRoomManager::roomProcess(GameRoom * room)
	{
		switch (room->GetState())
		{
		case RoomState::Waiting:
			#pragma region WAITING ROOM PROCESS

			// �濡 �����ڰ� �� ���ִٸ�, ������ ������ �˸��� ��Ŷ�� ������.
			if (room->GetPlayerCount() >= 2 && room->_isGameStartPacketSended == false)
			{
				// �÷��̾� ������ ���� ����.
				//auto player1Pos = Util::GetRandomNumber(-100, 100);
				//auto player2Pos = Util::GetRandomNumber(-100, 100);
				auto player1Pos = 50;
				auto player2Pos = -50;

				// ��Ŷ�� �÷��̾� ���� ���.
				Packet::GameStartNotify notify1;
				notify1._playerNumber = 1;
				notify1._positionX = player1Pos;
				notify1._enemyPositionX = player2Pos;
				notify1._positionY = 300;
				notify1._enemyPositionY = 300;

				Packet::GameStartNotify notify2;
				notify2._playerNumber = 2;
				notify2._positionX = player2Pos;
				notify2._enemyPositionX = player1Pos;
				notify2._positionY = 300;
				notify2._enemyPositionY = 300;

				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_GameStartNotify, room->_player1->GetSessionIdx(), &notify1);
				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_GameStartNotify, room->_player2->GetSessionIdx(), &notify2);

				// ������ ������ �濡 ���.
				room->_player1Pos = player1Pos;
				room->_player2Pos = player2Pos;

				room->_isGameStartPacketSended = true;

				_logger->Write(LogType::LOG_DEBUG, "%s | Player 1 Position(%d), 2 Position(%d)", __FUNCTION__, player1Pos, player2Pos);
			}

			#pragma endregion
			break;

		case RoomState::StartGame:
			#pragma region START GAME
		{
			// ���� ���Ѵ�.
			auto turnCoin = Util::GetRandomNumber(0, 100);

			// ���� ��Ŷ�� ����� ���´�.
			// TODO :: �ٶ� ���� ������.
			Packet::TurnStartNotify turnStartNotify;
			Packet::EnemyTurnStartNotify enemyTurnStartNotify;

			// ���� ���� ����ϰ�, ���ڿ��� ��Ŷ�� �����ش�.
			if (turnCoin >= 50)
			{
				room->_turnPlayer = 1;

				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_TurnStartNotify, room->_player1->GetSessionIdx(), &turnStartNotify);
				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_EnemyTurnStartNotify, room->_player2->GetSessionIdx(), &enemyTurnStartNotify);
			}
			else
			{
				room->_turnPlayer = 2;

				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_TurnStartNotify, room->_player2->GetSessionIdx(), &turnStartNotify);
				Util::PushToSendQueue(_sendQueue, Packet::PacketId::ID_EnemyTurnStartNotify, room->_player1->GetSessionIdx(), &enemyTurnStartNotify);
			}

			// �� ���¸� InGame���� �ٲ��ش�.
			room->_state = RoomState::InGame;

		}
			#pragma endregion
			break;

		case RoomState::InGame:
			#pragma region INGAME ROOM PROCESS

			// TODO :: Ÿ�̸� Ŭ���� ���� �� �ð� �缭 �� ������.

			#pragma endregion
			break;

		case RoomState::EndGame:
			#pragma region ENDGAME ROOM PROCESS

			#pragma endregion
			break;
		}
	}
}