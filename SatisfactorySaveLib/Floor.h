#pragma once

#include "FactoryMachine.h"

#include <vector>
#include <memory>

namespace factorygame {

	class Floor {
	public:
	private:
		float _elevation = 0.0f;
		std::vector<std::unique_ptr<GameObject>> _objects;
	};

}