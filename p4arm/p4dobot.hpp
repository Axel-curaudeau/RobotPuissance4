
#pragma once


#include "dobot/DobotDll.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "setting.hpp"
#include "ff/ffunistring.hpp"
#include "ff/fftime.hpp"
using namespace std;
struct dobot;
enum class nArmStat { connected = 0, disconnected };
enum class nGripStat { released = 0, grabbing, off };

struct dobot
{
	//configuration
	setting config;
	nArmStat status = nArmStat::disconnected;
	ff::unistring port;
	int id = -1;

	uint32_t paramsTimeout = 3000;
	PTPJointParams paramsPTPJoint = { 0 };
	PTPCoordinateParams paramsPTPCoord = { 0 };
	PTPCommonParams paramsPTPCommon = { 0 };
	PTPJumpParams paramsPTPJump = { 0 };
	JOGJointParams paramsJOGJoint = { 0 };
	JOGCoordinateParams paramsJOGCoord = { 0 };
	JOGCommonParams paramsJOGCommon = { 0 };



	dobot();

	/// \brief Attempt to connect to the first dobot found in port list
	/// 
	/// \return True if the connection is succesful, false otherwise
	bool connect();

	/// \brief Disconnect the dobot
	void disconnect();

	/// \brief Check if the robot is connected
	/// 
	/// \return True if the robot is connected, false otherwise
	bool isConnected() const;

	/// \brief Send a command to re-calibrate the robot (the function is blocking until the robot has finished calibrating)
	///
	/// \return True if the command was executed, false otherwise
	bool calibrate();

	/// \brief Ping the robot to check if it is still connected
	///
	/// \return True if the robot is still connected, false otherwise
	bool ping();

	/// \brief Move the robot to a specified position (the function is blocking until the robot has finished moving) (WARNING: make sure the robot is calibrated)
	/// \detail Use getPosition() to obtain position values
	///
	/// \return True if the command was executed, false otherwise
	bool moveTo(ptpPos _pos);

	/// \brief Get the robot's current position
	///
	/// \return The robot's current position
	ptpPos getPosition();

	/// \brief Change the robot's gripper state
	/// \detail The robot will make a lot of noise if the gripper is not off
	///
	/// \param _grip: The state to switch to (release, grabbing, off)
	/// 
	/// \return True if the command was executed, false otherwise
	bool setGrip(nGripStat _grip);


	/// \brief Get the grip state (released, grabbing, off)
	///
	/// \return The gripper's state
	nGripStat getGrip();


	/// \brief Get the position above ammo
	///
	/// \param _x: [0, 1] Which ammo rack to target
	/// 
	/// \return The position
	ptpPos getHighAmmoPos(uint _x, uint _y);


	/// \brief Get the position to grab ammo
	///
	/// \param _x: [0, 1] Which ammo rack to target
	/// \param _y: [0, 3] Which specific slot to target
	/// 
	/// \return The position
	ptpPos getAmmoPos(uint _x, uint _y);

	/// \brief Get the position above the board
	/// 
	/// \return The position
	ptpPos getHighColumnPos(uint _column);

	/// \brief Get the position to place into a column
	///
	/// \param _column: [0, 6] The column to target
	/// 
	/// \return The position
	ptpPos getColumnPos(uint _column);

	/// \brief Get the offset position to "bump" the token into the column
	///
	/// \param _column: [0, 6] The column to target
	/// 
	/// \return The position
	static ptpPos getOffsetXColumnPos(ptpPos _pos);

	/// \brief Get the offset position to "lock" the token into the column
	///
	/// \param _column: [0, 6] The column to target
	/// 
	/// \return The position
	static ptpPos getOffsetZColumnPos(ptpPos _pos);

	/// \brief use all the position to test the current configuration
	void test();


private:
	/// \brief Send parameters such as speed and acceleration settings to the dobot
	void configure();

	/// \brief Check if the last command is succesful
	/// \param int _commStat: Number returned by Dobot API functions to represent a response
	/// \return [true: if the command was succesful] [false: otherwise]
	bool checkStat(int _commStat);

	/// \brief Sleep until the command is executed
	/// \param uint64_t _cmdIdx: the id of the command
	/// \return [true: if the command was succesful] [false: otherwise]
	bool sleepUntilCmdExecuted(uint64_t _cmdIdx);
};


dobot::dobot()
{}
bool dobot::connect()
{
	if (isConnected()) { return true; }

	// Detection:
	char dobotSearchStr[1024] = { 0 };
	uint dobotSearchCount = 0;
	dobotSearchCount = SearchDobot(dobotSearchStr, 1024);
	if (dobotSearchCount == 0) { status = nArmStat::disconnected; return false; }
	ff::log() << "Dobot ports: " << dobotSearchStr << "\n";


	// Parsing:
	ff::dynarray<ff::regex::match> dobotSearchPorts = ff::unistring(dobotSearchStr).findRegex("[^ ]+");
	if (dobotSearchPorts.size() == 0) { status = nArmStat::disconnected; return false; }


	// Connection:
	status = nArmStat::disconnected;
	for (uint i = 0; i < dobotSearchPorts.size(); i += 1)
	{
		int dobotId = -1;
		int connectStatus = ConnectDobot(std::string(dobotSearchPorts[i].text).c_str(), 115200, nullptr, nullptr, &dobotId);

		if (connectStatus == DobotConnect_NoError)
		{
			port = dobotSearchPorts[i].text;
			id = dobotId;
			status = nArmStat::connected;
			break;
		}
	}

	if (status == nArmStat::disconnected) { return false; }



	// Try to get parameters:
	GetJOGJointParams(id, &paramsJOGJoint);
	GetJOGCoordinateParams(id, &paramsJOGCoord);
	GetJOGCommonParams(id, &paramsJOGCommon);
	GetPTPJointParams(id, &paramsPTPJoint);
	GetPTPCoordinateParams(id, &paramsPTPCoord);
	GetPTPCommonParams(id, &paramsPTPCommon);
	GetPTPJumpParams(id, &paramsPTPJump);

	configure();

	return true;
}
void dobot::disconnect()
{
	DisconnectDobot(id);
	status = nArmStat::disconnected;
}
bool dobot::isConnected() const { return status == nArmStat::connected; }
void dobot::configure()
{
	if (status != nArmStat::connected) { return; }

	// JOG:
	for (uint i = 0; i < 4; i += 1) { paramsJOGJoint.acceleration[i] = 200.0f; paramsJOGJoint.velocity[i] = 200.0f; }
	for (uint i = 0; i < 4; i += 1) { paramsJOGCoord.acceleration[i] = 200.0f; paramsJOGCoord.velocity[i] = 200.0f; }
	paramsJOGCommon.accelerationRatio = 200.0f; paramsJOGCommon.velocityRatio = 200.0f;

	// PTP:
	for (uint i = 0; i < 4; i += 1) { paramsPTPJoint.acceleration[i] = 200.0f; paramsPTPJoint.velocity[i] = 200.0f; }
	paramsPTPCoord.rAcceleration = 200.0f; paramsPTPCoord.xyzAcceleration = 200.0f; paramsPTPCoord.rVelocity = 200.0f; paramsPTPCoord.xyzVelocity = 200.0f;
	paramsPTPCommon.accelerationRatio = 100.0f; paramsPTPCommon.velocityRatio = 100.0f;
	paramsPTPJump.jumpHeight = 10.0f; paramsPTPJump.zLimit = 20.0f;

	int stat;
	uint64_t cmdIdx;

	stat = SetCmdTimeout(id, paramsTimeout); if (!checkStat(stat)) { return; }
	stat = SetJOGJointParams(id, &paramsJOGJoint, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetJOGCoordinateParams(id, &paramsJOGCoord, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetJOGCommonParams(id, &paramsJOGCommon, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetPTPJointParams(id, &paramsPTPJoint, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetPTPCoordinateParams(id, &paramsPTPCoord, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetPTPCommonParams(id, &paramsPTPCommon, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetPTPJumpParams(id, &paramsPTPJump, true, &cmdIdx); if (!checkStat(stat)) { return; }
	stat = SetEndEffectorGripper(id, false, false, true, &cmdIdx); if (!checkStat(stat)) { return; }

	if (!sleepUntilCmdExecuted(cmdIdx)) { disconnect(); return; }

	calibrate();
}
bool dobot::calibrate()
{
	int stat;
	uint64_t cmdIdx;

	HOMECmd home;
	stat = SetHOMECmd(id, &home, true, &cmdIdx); if (!checkStat(stat)) { return false; }
	if (!sleepUntilCmdExecuted(cmdIdx)) { disconnect(); return false; }

	return true;
}
bool dobot::ping()
{
	if (status != nArmStat::connected) { return false; }

	Pose pose;
	if (!checkStat(GetPose(id, &pose))) { disconnect(); return false; }
	return true;
}
// Move to a target position
// [REQUIRES]: dobot.status == ready
// [BLOCKING]: returns when the robot has reached the position
bool dobot::moveTo(ptpPos _pos)
{
	if (status != nArmStat::connected) { return false; }

	PTPCmd cmdPTP;
	cmdPTP.ptpMode = PTPMOVJXYZMode;
	cmdPTP.x = _pos.x;
	cmdPTP.y = _pos.y;
	cmdPTP.z = _pos.z;
	cmdPTP.r = _pos.r;

	
	int stat;
	uint64_t cmdIdx;
	stat = SetPTPCmd(id, &cmdPTP, true, &cmdIdx); if (!checkStat(stat)) { return false; }

	return sleepUntilCmdExecuted(cmdIdx);
}
// Get the robot position
// [REQUIRES]: dobot.status == connected || dobot.status == ready
// [NON-BLOCKING]: the position is returned immediately, even while other commands are running
ptpPos dobot::getPosition()
{
	if (status != nArmStat::connected) { return ptpPos(0, 0, 0, 0); }

	Pose pose;
	if (!checkStat(GetPose(id, &pose))) { return ptpPos(0, 0, 0, 0); }
	return ptpPos(pose.x, pose.y, pose.z, pose.r);
}
// Set the grip
// [REQUIRES]: dobot.status == ready
// [NON-BLOCKING]: returns when the grip has changed
bool dobot::setGrip(nGripStat _grip)
{
	if (status != nArmStat::connected) { return false; }

	int stat;
	uint64_t cmdIdx;
	if (_grip == nGripStat::grabbing) { stat = SetEndEffectorGripper(id, true, true, true, &cmdIdx); if (!checkStat(stat)) { return false; } }
	else if (_grip == nGripStat::released) { stat = SetEndEffectorGripper(id, true, false, true, &cmdIdx); if (!checkStat(stat)) { return false; } }
	else if (_grip == nGripStat::off) { stat = SetEndEffectorGripper(id, false, true, true, &cmdIdx); if (!checkStat(stat)) { return false; } }
	else { return false; }

	bool success = sleepUntilCmdExecuted(cmdIdx);
	ff::sleep(200);
	return success;
}
nGripStat dobot::getGrip()
{
	if (status != nArmStat::connected) { return nGripStat::grabbing; }

	bool gripperEnabled;
	bool gripperStatus;
	GetEndEffectorGripper(id, &gripperEnabled, &gripperStatus);

	if (!gripperEnabled) { return nGripStat::off; }
	else if (gripperStatus) { return nGripStat::grabbing; }
	else { return nGripStat::released; }
}
ptpPos dobot::getHighAmmoPos(uint _x,uint _y)
{
	int index=0;
	index = 4 * _x + _y;
	//for getting the rotation depending on the side of the ammunitions
	float rotation = 0;
	if (index < 4)
		rotation = -90;
	else
		rotation = 90;

	//convert the x and y into a array index

	if (index < 8) {
		return ptpPos(config.amunition[index].x, config.amunition[index].y, 150, rotation);
	}

	std::cout << "[!] ERROR: getHighAmmoPos was given incorrect ammo position\n";
	abort();
}
ptpPos dobot::getAmmoPos(uint _x, uint _y)
{
	int index = 0;
	//convert the x and y into a array index
	index = 4 * _x + _y;
	if (index < 8) {
		cout << "position of ammunition : x:" <<config.amunition[index].x << " y:" << config.amunition[index].y << " z:" << config.amunition[index].z << " r:" << config.amunition[index].r << "\n";
		return config.amunition[index];
	}

	std::cout << "[!] ERROR: getAmmoPos was given incorrect ammo position\n";
	abort();
}
ptpPos dobot::getHighColumnPos(uint _column)
{
	int index = _column;
	if (index < 7) {
		return ptpPos(config.board[index].x, config.board[index].y, 150, 0);
	}
}
ptpPos dobot::getColumnPos(uint _column)
{
	int index = _column;
	if (index < 7) {
		return config.board[index];
	}

	std::cout << "[!] ERROR: getColumnPos was given incorrect column number\n";
	abort();
}

ptpPos dobot::getOffsetXColumnPos(ptpPos _pos)
{
	ptpPos modifiedPos = _pos;
	modifiedPos.x += 8.0f;
	return modifiedPos;
}

ptpPos dobot::getOffsetZColumnPos(ptpPos _pos)
{
	ptpPos modifiedPos = _pos;
	modifiedPos.z -= 12.0f;
	return modifiedPos;
}

bool dobot::checkStat(int _commStat)
{
	if (_commStat == DobotCommunicate_NoError) { return true; }
	// other values for _commStat: DobotCommunicate_BufferFull, DobotCommunicate_Timeout, DobotCommunicate_InvalidParams
	disconnect();
	return false;
}

bool dobot::sleepUntilCmdExecuted(uint64_t _cmdIdx)
{
	int stat;
	uint64_t currCmdIdx;

	stat = SetQueuedCmdStartExec(id); if (!checkStat(stat)) { return false; }
	stat = GetQueuedCmdCurrentIndex(id, &currCmdIdx); if (!checkStat(stat)) { return false; }
	while (currCmdIdx < _cmdIdx) { stat = GetQueuedCmdCurrentIndex(id, &currCmdIdx); if (!checkStat(stat)) { return false; } }
	stat = SetQueuedCmdStopExec(id); if (!checkStat(stat)) { return false; }

	return true;
}

void dobot::test() {

	setGrip(nGripStat::off);			 //
	for (int i = 0; i < 8; i++) {


		ptpPos boardPlacement;
		if (i < 7)
			boardPlacement = config.board[i];
		else
			boardPlacement = config.board[0];
		float rotation = 0;
		if (i < 4)
			rotation = -90;
		else
			rotation = 90;


		ptpPos highAmoPos = ptpPos(config.amunition[i].x, config.amunition[i].y, 150, rotation);//
		ptpPos highColumnPos = ptpPos(boardPlacement.x, boardPlacement.y, 150, 0);//

		//get to the position on top of the ammunition
		moveTo(highAmoPos);		  //
		moveTo(config.amunition[i]); //
		setGrip(nGripStat::grabbing);				  //
		moveTo(highAmoPos);		  // Move to the ammo and grab a token

		moveTo(highColumnPos);		   //
		moveTo(boardPlacement);	   //
		setGrip(nGripStat::off);						   //
		ff::sleep(500);											   //
		setGrip(nGripStat::released);
		ptpPos offsetZ = boardPlacement;//
		offsetZ.z -= 16.0f;
		moveTo(offsetZ);
		ptpPos offsetX = offsetZ;//
		offsetX.x += 8.0f;
		moveTo(offsetX); // (<- this line is used to "bump" the token into the column)
		ff::sleep(500);
		moveTo(highColumnPos);			   // Move to the column and drop the token

		setGrip(nGripStat::off);			 //
	}
	moveTo(ptpPos(-57.5f, -226.5f, 150.0f, -90.0f)); // Turn grip off and place in a safer position

}

