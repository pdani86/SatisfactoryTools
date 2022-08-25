#pragma once

#include "FactoryMachine.h"

#include <vector>
#include <memory>

namespace factorygame {

	class Floor {
	public:
		void addObject(std::unique_ptr<GameObject> object);

	private:
		float _elevation = 0.0f;
		int _foundationWidth = 10;
		int _foundationHeight = 10;
		std::vector<std::unique_ptr<GameObject>> _objects;
	};

	class FactoryBuilding {
	public:
		void addFloor(int foundationWidth, int foundationHeight);
		void addFloor(std::unique_ptr<Floor> floor);
		Floor& getFloor(int ix);
		int floorCount() const { return _floors.size(); }

	private:
		std::vector<std::unique_ptr<Floor>> _floors;
	};


	class FactoryBuildingBuilderExample {
	public:
		static std::unique_ptr<FactoryBuilding> build();
	};

}
