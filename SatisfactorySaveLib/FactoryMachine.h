#pragma once

namespace factorygame {

	class GameObject {
	public:
		virtual ~GameObject() = default;
	};

	class FactoryMachineBase : public GameObject {
	public:
		virtual ~FactoryMachineBase() = default;
	private:

	};

	class Constructor : public FactoryMachineBase {
	public:
		static constexpr float width = 800.0f;
		static constexpr float length = 1000.0f;
		static constexpr float height = 800.0f;
	};

}
