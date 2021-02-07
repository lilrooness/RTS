#pragma once

#include <vector>
#include <glm.hpp>
#include <cstdlib>

struct IndexReference;
struct Index;
struct MouseDragData;
struct Tank;
struct TanksData;
struct Game;

bool test2DRect(glm::vec2 point, glm::vec2 bottomLeft, float width, float height);
bool XZPointWithinRect(glm::vec3 p1, glm::vec3 r1, glm::vec3 r2);

bool makeQuad(glm::vec3 a, glm::vec3 b, GLfloat* quadVertexBuffer_out);
bool rayGroundPlaneIntersection(glm::vec3 rayDirection, glm::vec3 rayStart, glm::vec3* answer);
IndexReference addTank(Game* game, float x, float y, float z, int health);

const glm::vec4 DEFAULT_COLOR = glm::vec4(0.3f, 0.1f, 0.1f, 1.0f);

vector<int> getNeighbourCellIndexes(Game* game, int cellIndex);
int realCoordsToMapIndex(Game* game, float x, float y);
float getFScoreForGidPoint(Game* game, int currentCellIndex, int neighbourCellIndex, int waypointCellIndex);
int mapCoordsToMapIndex(Game* game, int x, int y);
void mapIndexToMapCoords(Game* game, int mapIndex, int* coordsOut);
void mapIndexToRealCorrds(Game* game, int mapIndex, float* coordsOut);

struct flowCell {
	//float density{ 0.0f };
	int discomfort{ 0 };
	int x;
	int y;
};

struct flockingWeights {
	float allignment{ 0.05f };
	float cohesion{ 0.0f };
	float seperation{ 0.6f };
	float radius{ 20.0f };
	float seperationRadius{ 5.0f };
};

struct IndexReference {
	int generation;
	int index;
};

struct Index {
	int generation{0};
	bool deleted{ false };
};

struct MouseDragData {
	glm::vec3 origin;
	glm::vec3 drag;
};

struct Waypoint {
	glm::vec3 point;
	bool set{ false };
};

// Tank game data
struct Tank {
	Index index;
	int health;
	glm::vec3 direction;
	float speed{0.1};
	bool selected{ false };
	Waypoint waypoint;
};

// All Tanks Rendering Data (buffers)
struct TanksData {
	std::vector<GLfloat> positions;
	std::vector<float> headings;
	std::vector<float> turretDirections;
	std::vector<float> tint;
};

struct Game {
	std::vector<Tank> tanks;
	TanksData tanksData;
	MouseDragData mouseDragData;
	bool primaryButtonDown{ false };
	bool secondaryButtonDown{ false };
	bool secondaryButtonClicked{ false };
	glm::vec3 currentMouseGroundIntersection;

	GLfloat groundSelectionQuadVertices[12]{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
	}; //length 12 (4 points)

	Settings settings;
	flockingWeights tankFlockingWeights;
	std::vector<flowCell> flowCells;
	int flowMapWidth{ 300 };
	int flowMapHeight{ 300 };
	float flowCellSize{ 2.0f };
};

float getFScoreForGidPoint(Game *game, int currentCellIndex, int neighbourCellIndex, int waypointCellIndex) {
	auto currentCell = game->flowCells[currentCellIndex];
	auto waypointCell = game->flowCells[waypointCellIndex];
	auto neighbourCell = game->flowCells[neighbourCellIndex];
	
	int waypointMapCoords[2];
	mapIndexToMapCoords(game, waypointCellIndex, waypointMapCoords);

	int neighbourMapCoords[2];
	mapIndexToMapCoords(game, neighbourCellIndex, neighbourMapCoords);

	float dist = pow(waypointMapCoords[0] - neighbourMapCoords[0], 2) + pow(waypointMapCoords[1] - neighbourMapCoords[1], 2);

	int discomfort = currentCell.discomfort - neighbourCell.discomfort;

	return dist;//  +discomfort;
}

int getBestNeighbouringCellIndex(Game* game, IndexReference tankReference, Waypoint waypoint) {
	float tankX = game->tanksData.positions[3 * tankReference.index];
	float tankY = game->tanksData.positions[3 * tankReference.index + 1];
	float tankZ = game->tanksData.positions[3 * tankReference.index + 2];
	int tankCurrentCellIndex = realCoordsToMapIndex(game, tankX, tankZ);
	int waypointIndex = realCoordsToMapIndex(game, waypoint.point.x, waypoint.point.z);

	vector<int> neighbouringCellIndexes = getNeighbourCellIndexes(game, tankCurrentCellIndex);
	vector<float> neighbourScores;

	for (int i = 0; i < neighbouringCellIndexes.size(); i++) {
		neighbourScores.push_back(getFScoreForGidPoint(game, tankCurrentCellIndex, neighbouringCellIndexes[i], waypointIndex));
	}

	float lowestScore = 0;
	int lowestScoreIndex = -1;

	for (int i = 0; i < neighbourScores.size(); i++) {
		if (lowestScoreIndex == -1) {
			lowestScoreIndex = i;
			lowestScore = neighbourScores[i];
		}
		else if (lowestScore > neighbourScores[i]) {
			lowestScoreIndex = i;
			lowestScore = neighbourScores[i];
		}
	}
	return neighbouringCellIndexes[lowestScoreIndex];
}

vector<int> getNeighbourCellIndexes(Game* game, int cellIndex) {
	vector<int> cellIndexes;

	int cellCoords[2];
	mapIndexToMapCoords(game, cellIndex, cellCoords);

	int above = mapCoordsToMapIndex(game, cellCoords[0], cellCoords[1] + 1);
	int below = mapCoordsToMapIndex(game, cellCoords[0], cellCoords[1] - 1);
	int right = mapCoordsToMapIndex(game, cellCoords[0] + 1, cellCoords[1]);
	int left = mapCoordsToMapIndex(game, cellCoords[0] - 1, cellCoords[1]);

	int topRight = mapCoordsToMapIndex(game, cellCoords[0] + 1, cellCoords[1] + 1);
	int topLeft = mapCoordsToMapIndex(game, cellCoords[0] - 1, cellCoords[1] + 1);
	int bottomLeft = mapCoordsToMapIndex(game, cellCoords[0] - 1, cellCoords[1] - 1);
	int bottomRight = mapCoordsToMapIndex(game, cellCoords[0] + 1, cellCoords[1] - 1);


	if (bottomLeft > -1) {
		cellIndexes.push_back(bottomLeft);
	}
	if (bottomRight > -1) {
		cellIndexes.push_back(bottomRight);
	}
	if (topLeft > -1) {
		cellIndexes.push_back(topLeft);
	}
	if (topRight > -1) {
		cellIndexes.push_back(topRight);
	}


	if (above > -1) {
		cellIndexes.push_back(above);
	}
	if (below > -1) {
		cellIndexes.push_back(below);
	}
	if (right > -1) {
		cellIndexes.push_back(right);
	}
	if (left > -1) {
		cellIndexes.push_back(left);
	}

	return cellIndexes;
}

void initFlowMap(Game *game) {
	for (int h = 0; h < game->flowMapHeight; h++) {
		for (int w = 0; w < game->flowMapWidth; w++) {
			flowCell cell;
			cell.x = w;
			cell.y = h;
			game->flowCells.push_back(cell);
		}
	}
}

int mapCoordsToMapIndex(Game* game, int x, int y) {
	int index = game->flowMapWidth * y + x;

	if (index < 0) {
		return -1;
	}
	else if (index >= game->flowCells.size()) {
		return -1;
	}

	return index;
}

void mapIndexToMapCoords(Game* game, int mapIndex, int *coordsOut) {
	int xi = mapIndex % game->flowMapWidth;
	int yi = mapIndex / game->flowMapWidth;

	coordsOut[0] = xi;
	coordsOut[1] = yi;
}

int realCoordsToMapIndex(Game *game, float x, float y) {

	//calculate real size of map
	float realMapWidth = game->flowCellSize * game->flowMapWidth;
	float realMapHeight = game->flowCellSize * game->flowMapHeight;

	//transform coordinates so that 0,0 is at the "bottom left", not in the middle
	x += realMapWidth / 2.0f;
	y += realMapHeight / 2.0f;

	int xi = x / game->flowCellSize;
	int yi = y / game->flowCellSize;

	int index = game->flowMapWidth * yi + xi;

	if (index >= game->flowCells.size() || index < 0) {
		return -1;
	}

	return index;
}

void mapIndexToRealCorrds(Game* game, int mapIndex, float* coordsOut) {
	int xi = mapIndex % game->flowMapWidth;
	int yi = mapIndex / game->flowMapWidth;

	//calculate real size of map
	float realMapWidth = game->flowCellSize * game->flowMapWidth;
	float realMapHeight = game->flowCellSize * game->flowMapHeight;

	coordsOut[0] = (float)xi * game->flowCellSize - realMapWidth / 2.0f;
	coordsOut[1] = (float)yi * game->flowCellSize - realMapHeight / 2.0f;
}

bool validTankRef(IndexReference tankRef, Game* game) {
	if (tankRef.generation != game->tanks[tankRef.index].index.generation || game->tanks[tankRef.index].index.deleted) {
		return false;
	}

	return true;
}

void tickTank(IndexReference tankRef, Game *game) {
	
	if (!validTankRef(tankRef, game)) {
		std::cout << "invalid tank reference" << std::endl;
		return;
	}

	glm::vec3 pos(1.0f);
	pos.x = game->tanksData.positions[3 * tankRef.index];
	pos.y = game->tanksData.positions[3 * tankRef.index + 1];
	pos.z = game->tanksData.positions[3 * tankRef.index + 2];

	Tank& tank = game->tanks[tankRef.index];

	if (tank.waypoint.set) {
		if (glm::length(pos - tank.waypoint.point) < 1) {
			tank.waypoint.set = false;
		} else {

			int currentTankCellIndex = realCoordsToMapIndex(game, pos.x, pos.z);
			float realCurrentCellCoords[2];
			mapIndexToRealCorrds(game, currentTankCellIndex, realCurrentCellCoords);

			int cellIndex = getBestNeighbouringCellIndex(game, tankRef, tank.waypoint);

			float nextWaypointCoords[2];
			mapIndexToRealCorrds(game, cellIndex, nextWaypointCoords);

			
			float newHeading = -1.0f * atan2(nextWaypointCoords[0] - realCurrentCellCoords[0], nextWaypointCoords[1] - realCurrentCellCoords[1]);

			game->tanksData.headings[tankRef.index] = newHeading;

			glm::vec4 newDirection =
				glm::vec4(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f) *
				glm::rotate(
					glm::mat4(1.0),
					game->tanksData.headings[tankRef.index],
					glm::vec3(0.0f, 1.0f, 0.0f)
				);

			newDirection = glm::normalize(newDirection) * tank.speed;

			tank.direction.x = newDirection.x;
			tank.direction.y = newDirection.y;
			tank.direction.z = newDirection.z;

			//advance the tank by the new direction * speed
			game->tanksData.positions[3 * tankRef.index] += tank.direction.x;
			game->tanksData.positions[3 * tankRef.index + 1] += tank.direction.y;
			game->tanksData.positions[3 * tankRef.index + 2] += tank.direction.z;
		}

	}
}

//assumes a and b have lie on the x,z ground plane (have y coord of zero)
bool makeQuad(glm::vec3 a, glm::vec3 b, GLfloat* quadVertexBuffer_out) {
	if (a == b) {
		return false;
	}

	auto c = glm::vec3(a.x, 0.0f, b.z);
	auto d = glm::vec3(b.x, 0.0f, a.z);
	
	//In order of indices (cannot know actual position)
	//bottom left
	quadVertexBuffer_out[0] = a.x;
	quadVertexBuffer_out[1] = a.y;
	quadVertexBuffer_out[2] = a.z;

	//bottom right
	quadVertexBuffer_out[3] = c.x;
	quadVertexBuffer_out[4] = c.y;
	quadVertexBuffer_out[5] = c.z;
	
	//top right
	quadVertexBuffer_out[6] = b.x;
	quadVertexBuffer_out[7] = b.y;
	quadVertexBuffer_out[8] = b.z;
	
	//top left
	quadVertexBuffer_out[9] = d.x;
	quadVertexBuffer_out[10] = d.y;
	quadVertexBuffer_out[11] = d.z;

	return true;
}

//We assume that the ray starts behind the camera in NDC
bool rayGroundPlaneIntersection(glm::vec3 rayDirection, glm::vec3 rayStart, glm::vec3 *answer) {
	auto planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);

	float denom = glm::dot(planeNormal, rayDirection);

	if (denom == 0.0f) {
		return false;
	}

	float t = -(glm::dot(planeNormal, rayStart)) / (glm::dot(planeNormal, rayDirection));

	*answer = rayStart + (t * rayDirection);
	return true;
}

void resetSelectionQuadVertices(Game* game) {
	for (int i = 0; i < 12; i++) {
		game->groundSelectionQuadVertices[i] = 0.0f;
	}
}

void tick(Game* game) {
	
	//if we are dragging, then update the drag square data
	if (game->primaryButtonDown) {
		makeQuad(game->mouseDragData.origin, game->mouseDragData.drag, game->groundSelectionQuadVertices);
	}

	int tanksSelected = 0;

	for (int i=0; i < game->tanks.size(); i++) {
		Tank* tank = &game->tanks[i];

		tickTank(IndexReference{ tank->index.generation, i }, game);

		if (game->primaryButtonDown) {
			auto pos = glm::vec3(game->tanksData.positions[i * 3], 0.0f, game->tanksData.positions[(i * 3) + 2]);

			//quad verteces
			auto p1 = glm::vec3(game->groundSelectionQuadVertices[0], game->groundSelectionQuadVertices[1], game->groundSelectionQuadVertices[2]);
			auto p2 = glm::vec3(game->groundSelectionQuadVertices[3], game->groundSelectionQuadVertices[4], game->groundSelectionQuadVertices[5]);
			auto p3 = glm::vec3(game->groundSelectionQuadVertices[6], game->groundSelectionQuadVertices[7], game->groundSelectionQuadVertices[8]);
			auto p4 = glm::vec3(game->groundSelectionQuadVertices[9], game->groundSelectionQuadVertices[10], game->groundSelectionQuadVertices[11]);

			//divide the rectagle into two triangles and test each one seperately
			if (XZPointWithinRect(pos, game->mouseDragData.origin, game->mouseDragData.drag)) {
				game->tanksData.tint[(i * 4)] = 0.1;
				game->tanksData.tint[(i * 4) + 1] = 0.3;
				game->tanksData.tint[(i * 4) + 2] = 0.1;
				game->tanksData.tint[(i * 4) + 3] = 1.0;
				tank->selected = true;
			}
			else {
				game->tanksData.tint[(i * 4)] = DEFAULT_COLOR.x;
				game->tanksData.tint[(i * 4) + 1] = DEFAULT_COLOR.y;
				game->tanksData.tint[(i * 4) + 2] = DEFAULT_COLOR.z;
				game->tanksData.tint[(i * 4) + 3] = DEFAULT_COLOR.w;
				tank->selected = false;
			}
		} else if (game->secondaryButtonClicked && tank->selected) {
			game->tanks[i].waypoint.point = game->currentMouseGroundIntersection;
			game->tanks[i].waypoint.set = true;
		}
	}

	game->secondaryButtonClicked = false;
}

IndexReference addTank(Game* game, float x, float y, float z, int health) {
	IndexReference reference;
	bool tankCreated = false;
	int newIndex = -1;
	int generation = 0;
	for (int i = 0; i < game->tanks.size(); i++) {
		Tank& tank = game->tanks[i];
		if (tank.index.deleted) {
			newIndex = i;
			tank.index.deleted = false;
			generation = tank.index.generation++;
			tank.health = health;
			tankCreated = true;
			break;
		}
	}

	if (newIndex == -1) {
		newIndex = game->tanks.size();
	}

	reference.index = newIndex;
	reference.generation = generation;

	float heading = glm::radians(float(rand() % 360));
	//float heading = 0.0f;
	float turredDirection = 0.0f;

	if (!tankCreated) {
		tankCreated = true;
		Tank tank;
		//CHECK: Using defaults for tank.index
		tank.health = health;
		game->tanks.push_back(tank);

		game->tanksData.positions.push_back(x);
		game->tanksData.positions.push_back(y);
		game->tanksData.positions.push_back(0.0f);
		game->tanksData.headings.push_back(heading);
		game->tanksData.turretDirections.push_back(turredDirection);
		game->tanksData.tint.push_back(DEFAULT_COLOR.x);
		game->tanksData.tint.push_back(DEFAULT_COLOR.y);
		game->tanksData.tint.push_back(DEFAULT_COLOR.z);
		game->tanksData.tint.push_back(DEFAULT_COLOR.w);
	} else {
		game->tanksData.positions[reference.index] = x;
		game->tanksData.positions[reference.index + 1] = y;
		game->tanksData.positions[reference.index + 2] = 0.0f;
		game->tanksData.headings[reference.index] = heading;
		game->tanksData.turretDirections[reference.index] = turredDirection;
		game->tanksData.tint[reference.index * 4] = DEFAULT_COLOR.x;
		game->tanksData.tint[reference.index * 4 + 1] = DEFAULT_COLOR.y;
		game->tanksData.tint[reference.index * 4 + 2] = DEFAULT_COLOR.z;
		game->tanksData.tint[reference.index * 4 + 3] = DEFAULT_COLOR.w;
	}

	return reference;
}

bool XZPointWithinRect(glm::vec3 p1, glm::vec3 r1, glm::vec3 r2) {
	
	bool hit = false;
	if (r1.x > r2.x) {
		if (r1.z > r2.z) {
			//r1 is top right, r2 is bottom left
			hit = test2DRect(glm::vec2(p1.x, p1.z), glm::vec2(r2.x, r2.z), r1.x - r2.x, r1.z - r2.z);
		}
		else {
			//r1 is bottom right, r2 is top left
			hit = test2DRect(glm::vec2(p1.x, p1.z), glm::vec2(r2.x, r1.z), r1.x - r2.x, r2.z - r1.z);
		}
	}
	else {
		if (r1.z > r2.z) {
			//r1 is top left, r2 is bottom right
			hit = test2DRect(glm::vec2(p1.x, p1.z), glm::vec2(r1.x, r2.z), r2.x - r1.x, r1.z - r2.z);
		}
		else {
			//r1 is bottom left, r2 is top right
			hit = test2DRect(glm::vec2(p1.x, p1.z), glm::vec2(r1.x, r1.z), r2.x - r1.x, r2.z - r1.z);
		}
	}

	return hit;
}

bool test2DRect(glm::vec2 point, glm::vec2 bottomLeft, float width, float height) {
	if (point.x >= bottomLeft.x && point.x <= bottomLeft.x + width && point.y > bottomLeft.y && point.y <= bottomLeft.y + height) {
		return true;
	}

	return false;
}