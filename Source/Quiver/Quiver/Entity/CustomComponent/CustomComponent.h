#pragma once

#include "Quiver/Entity/Component.h"

#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include "json.hpp"

namespace sf {
class Window;
}

namespace qvr {

class CustomComponentType;
class RawInputDevices;

// This type of Component defines behaviour for the Entity it is attached to.
// Once per world step, the CustomComponent's OnStep method is called.
class CustomComponent : public Component {
public:
	CustomComponent(Entity& entity);

	virtual ~CustomComponent();

	CustomComponent(const CustomComponent&) = delete;
	CustomComponent(const CustomComponent&&) = delete;

	CustomComponent& operator=(const CustomComponent&) = delete;
	CustomComponent& operator=(const CustomComponent&&) = delete;

	virtual nlohmann::json ToJson() const { return nlohmann::json(); };
	virtual bool FromJson(const nlohmann::json& j) { return true; }

	// Override this with per-frame behaviour.
	// TODO: Make this take a const std::chrono::seconds (should be float seconds not integral tho)
	virtual void OnStep(float timestep) {}

	virtual void HandleInput(qvr::RawInputDevices& inputDevices, const float deltaSeconds) {}

	virtual void OnBeginContact(Entity& other) {}
	virtual void OnEndContact(Entity& other) {}

	// Override this with ImGui calls.
	virtual void GUIControls() {}

	// Override this to return the type name string of your subclass.
	virtual std::string GetTypeName() const = 0;

	bool GetRemoveFlag() const { return mRemoveFlag; }

protected:
	// Signal to the World that this Entity should be removed.
	void SetRemoveFlag(const bool removeFlag) { mRemoveFlag = removeFlag; }

private:
	bool mRemoveFlag = false;
};

// Instantiate this to register a subclass of CustomComponent.
// You will need to provide a string name for the type, a factory function
// and a JSON-verification function.
class CustomComponentType final {
public:
	CustomComponentType(
		const std::string typeName,
		std::function<std::unique_ptr<CustomComponent>(Entity&)> factoryFunc,
		std::function<bool(const nlohmann::json&)> verifyJsonFunc);

	CustomComponentType(const CustomComponentType&) = delete;
	CustomComponentType(const CustomComponentType&&) = delete;

	CustomComponentType& operator=(const CustomComponentType&) = delete;
	CustomComponentType& operator=(const CustomComponentType&&) = delete;

	std::unique_ptr<CustomComponent> CreateInstance(Entity& entity) {
		return mFactoryFunc(entity);
	}

	bool VerifyJson(const nlohmann::json& j) const {
		return mVerifyJsonFunc(j);
	}

	std::unique_ptr<CustomComponent> CreateInstance(Entity& entity, const nlohmann::json& j);

	std::string GetName() const { return mName; };

private:
	std::string mName;
	std::function<std::unique_ptr<CustomComponent>(Entity&)> mFactoryFunc;
	std::function<bool(const nlohmann::json&)> mVerifyJsonFunc;
};

class CustomComponentTypeLibrary {
public:
	bool RegisterType(std::unique_ptr<CustomComponentType> type);
	bool ForgetType(const std::string typeName);

	bool TypeExists(const std::string typeName) const;

	CustomComponentType* GetType(const std::string typeName) const;

	std::vector<std::string> GetTypeNames() const;

	std::unique_ptr<CustomComponent> CreateInstance(
		Entity& entity,
		const nlohmann::json& j) const;

	bool IsValid(const nlohmann::json& j) const;

private:
	std::unordered_map<std::string, std::unique_ptr<CustomComponentType>> mTypes;
};

}