#pragma once
#include"framework_base.h"
#include"MyList.h"

constexpr int PEOPLE_MAX = 500;
constexpr int ELEVATOR_MAX = 8;
constexpr int ELEVATOR_MIN = 2;

#define DISTANCE(x1,y1,x2,y2) sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))

extern bool continuedSimulation;
extern bool continuedAllow;
class NPC_Passenger;
class Floor;
class Building;
class Statistics;
constexpr int FLOOR_NUM=10;

constexpr float ROOM_STARNDARD_WIDTH = 3000;

struct Camera_Shake_Info
{
	float shake_dx_max, shake_dy_max;
	float shake_progress;
	float shake_time;
};
class Camera
{
public:
	Camera(float posx, float posy);
	~Camera();
	
	//rending range
	//float edge_x1, edge_x2, edge_y1, edge_y2;
	void moveTo(float x, float y,bool immediate=0);
	void moveDelta(float dx, float dy, bool immediate = 0);
	float getZoom();
	void setZoom(float zoom);
	void update();

	//transform to viewpoint position
	//side 0 for x, 1 for y
	inline void to_view(float* size);
	inline void to_viewX(float* in_mapx);
	inline void to_viewY(float* in_mapy);

	bool Check_in_View(float in_map_posx, float in_map_posy, float range_size);
	bool Check_in_ViewY(float in_map_posy, float range_size);
	void shakeCamera(float shake_force, float shake_time, float shake_angle);

private:
	float posx, posy;
	float zoom;
	float posx_ori, posy_ori;
	float speedx, speedy;
	float movingTime;
	float zoom_speed;
	float zoomChangeTime;
	Camera_Shake_Info* shakes[32];
	const float shake_limit = 50;
};

inline void Camera::to_view(float* size)
{
	*size *= zoom;
	return;
}

inline void Camera::to_viewX(float* in_mapx)
{
	*in_mapx = 800 + (*in_mapx - posx) * zoom;
	return;
}

inline void Camera::to_viewY(float* in_mapy)
{
	*in_mapy = 450 + (*in_mapy - posy) * zoom;
	return;
}

class Elevator
{
public:
	Elevator(int id, Building* context);
	~Elevator();
	enum class Stutas
	{
		IDLE = 0,
		Parking = 1,
		RunningUP = 2,
		RunningDOWN = 3,
		CLOSED = 4,
		CLOSING = 5
	};

	enum class DoorStutas
	{
		Opening = 0,
		Opened = 1,
		Closing = 2,
		Closed = 3
	};

	enum class MsgType
	{
		ArrivedFloor = 0,
		ArrivingFloor = 1,
		LeftFloor = 2
	};


	//be called by floor object or passenger
	void AddNPC_Passenger(NPC_Passenger* newPassenger);
	void DelNPC_Passenger(NPC_Passenger* newPassenger);

	bool getAttendable();	//be called by passenger
	void RequestLevel(int level);	//be called by passenger

	int getFloorPos();
	bool isDirectionUp();

	Stutas getStutas();
	void setStutas(Stutas newStutas);

	void UpdateTargetFloor();

	void update(float timeFactor);
	void rend(Camera* viewPort);
	static constexpr int Capacity = 24;
	static constexpr float ELEVATOR_HEIGHT = 3000;
	static constexpr float ELEVATOR_WIDTH = 1800;
private:
	
	static constexpr float Parking_Distance_Min = 50;
	static constexpr float RUNNING_Speed = 1000;
	static constexpr float Door_Speed = 1;
	static constexpr float DoorClosingCD = 10;
	static constexpr float IDLEBoardCastCD = 3;
	static constexpr float ParkingBoardCastCD = 1;
	static constexpr float REFRESH_CD = 1;
	LinkList<NPC_Passenger*> *passengers;
	
	Building* context;
	Stutas stutas;
	DoorStutas doorStutas;
	int id;	//1-10
	int floorPos;	//1-10
	int currenttargetFloor;
	int peopleNum;
	float posx;
	float posy;
	float doorOpenPos;	//-0.5-1
	float doorCloseCD;
	bool runningDirectionUP;

	float boardcastCD;
	float refreshCD;
	bool requestedLevels[FLOOR_NUM+1];
};


class NPC_Render
{
public:
	NPC_Render(D2D1_COLOR_F color);
	void setOpacity_factor(float factor);
	//prepare position data
	void setStill();
	void update(float timeFactor, float posx, float posy,bool still=0);
	void rend(Camera* viewPort);
	static constexpr float head_size = 400;
private:
	
	//body stutas
	static constexpr float ARM_angel_max = 0.25f * PI;
	static constexpr float LEG_angel_max = 0.1f * PI;
	static constexpr float ARM_RADIUS = 400;
	static constexpr float LEG_RADIUS = 800;

	D2D1_COLOR_F color;
	float arm_angel;
	float leg_angel;
	float opacity_factor;

	//pos
	float center_x;
	float arm1_x;
	float arm2_x;
	float leg1_x;
	float leg2_x;
	float heady;
	float body1_y;
	float body2_y;
	float arm_y;
	float leg_y;
};
class NPC_Passenger
{
public:
	NPC_Passenger(Building* contextBuilding, int type);
	~NPC_Passenger();
	enum class Stutas
	{
		WalkingToElevator = 0,
		WalkingToRoom = 1,
		WaitingElevator = 2,
		TakingElevator = 3,
		InRoom = 4,
		WalkingToArrivingElevator = 5,
		WalkingToStair = 7,
		InStair = 8
	};
	Stutas getStutas();
	void setStutas(Stutas newStutas);
	void setPos(float x, float y);
	void setTargetX(float x);
	void update(float timeFactor);
	void rend(Camera* viewPort);
	void ReceiveElevatorMsg(int ElevatorID, Elevator::MsgType ElevatorStutas);

	void getOnElevator(int elevatorID);
	void getOffElevator();

	float getWaitTime();

	static constexpr float Interact_Distance_Min = 300;
private:
	static constexpr float Standard_Speed = 1000;
	static constexpr float WAITING_TIME_MIN = 120;
	static constexpr float WAITING_TIME_MAX = 240;
	Stutas stutas;
	float inRoomCD;
	float posx, posy;
	float target_posx;
	
	float speedX;
	float maxSpeedX;
	float waitingCD;
	float waitTime;

	int target_elevator_id;
	int currentLevel;
	int targetLevel;
	Floor* floor;
	Building* context;
	
	NPC_Render* render;

	enum class Type
	{
		Red=0,
		Green=1,
		Blue=2,
		Purple=3
	};
	Type type;
	enum class Density
	{
		LOW = 0,
		MID = 1,
		HIGH = 2,
	};
	static Density density;
};

class Floor
{
public:
	static constexpr float FLOOR_HEIGHT = 3000;
	static constexpr float FLOOR_WIDTH = 36000;
	Floor(Building* context,int level);
	~Floor();
	void update(float timeFactor);
	void rend(Camera* viewPort);
	int getNearestElevatorID();
	int getRandomElevatorID();
	bool requestElevator(bool isUP);
	LinkList<int>* getArrivingElevator();
	float getPosy();
	float getRoomPosx();
	void BoardcastElevatorMsg(int ElevatorID, Elevator::MsgType ElevatorStutas);

	void AddNPC_Passenger(NPC_Passenger* newPassenger);
	void DelNPC_Passenger(NPC_Passenger* newPassenger);
private:
	
	LinkList<NPC_Passenger*>* passengers;	//use mylist
	Building* context;
	int level;
	int peopleNumInHallway;
	int peopleNumInRoom;
	int ElevatorNum;
	float elevator_posx1[ELEVATOR_MAX], elevator_posx2[ELEVATOR_MAX];
	float elevator_posy1;
	float posx, posy;
	float posx1, posx2, posy1, posy2;
	float ElevatorPosxBuff;
	bool requestUP, requestDown;
	LinkList<int>* arrivingElevatorList;

	//rending
	float requestUPEffectTime, requestDownEffectTime;
};




//target area 36000x30000
class Building
{
public:
	static int ui_level;

	Building();
	~Building();
	void update();
	void rend();
	Floor* getFloor(int level);	//1-10
	Elevator* getElevator(int id);	//1-10
	bool getElevatorAvailable(int index);//0-(max-1)
	int getElevatorNum();

	void AddRequestFloor(int level, bool isUp);
	void DelRequestFloor(int level, bool isUp);

	LinkList<int>* getUpRequestFloors();
	LinkList<int>* getDownRequestFloors();
	bool getFloorUpRequest(int level);
	bool getFloorDownRequest(int level);

	void setTimeFactor(bool isUp);
	void setPause();
	bool getPaused();

	//todo: add msg methed
	//void setMethod(int newMethod);
	/*LinkList<int>* PeekUpRequestFloors();
	LinkList<int>* PeekDownRequestFloors();
	int GetUpRequestFloor(int currentLevel);
	int GetDownRequestFloor(int currentLevel);*/
	static constexpr int FLOOR_NUM = FLOOR_NUM;
private:
	static constexpr int CAMERA_SPEED = 3000;
	static constexpr float TICKTIME_MIN = 0.0625f;
	
	int peopleNumTotal;
	int ElevatorNum;
	float timeFactor;
	bool isPaused;


	LinkList<int>* UpRequestFloors,*DownRequestFloors;
	
	NPC_Passenger* passengers[PEOPLE_MAX];
	Elevator* elevators[ELEVATOR_MAX];
	Floor* floors[FLOOR_NUM];
	Camera* viewPort;
	Statistics* dataBoard;
};


class Statistics
{
public:
	Statistics(LinkList<NPC_Passenger*>* passengers, LinkList<Elevator*>* elevators);
	~Statistics();
	void update();
	void rend();
private:
	LinkList<NPC_Passenger*>* passengersList;
	LinkList<Elevator*>* elevatorsList;
	float AvgRunningElevatorUsage;
	float AvgTotalElevatorUsage;
	float AvgPassengerWatingTime;
	float MaxPassengerWatingTime;
	float AvgPassengerWatingNum;
	int MaxPassengerWatingNum;

	int waitingPassengerNum;
	int walkingPassengerNum;
	int InElevatorPassengerNum;
	int InRoomPassengerNum;
};