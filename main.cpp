#include "./includes.h"
#include "./FastPatch.h"

struct ColorPatch {
	size_t rAddress = 0;
	size_t gAddress = 0;
	size_t bAddress = 0;
	size_t aAddress = 0;
	std::string hexColor;

	ColorPatch(const std::string& hexColor, size_t r, size_t g, size_t b, size_t a)
		: hexColor(hexColor), rAddress(r), gAddress(g), bAddress(b), aAddress(a) {}

	ColorPatch(const std::string& hexColor, size_t all)
		: hexColor(hexColor), rAddress(all), gAddress(all + 1), bAddress(all + 2), aAddress(all + 3) {}

	void apply() {
		if (hexColor.size() == 6) hexColor = hexColor + "FF";

		if (hexColor.size() != 8) {
			std::cout << "wrong color format" << std::endl;
			return;
		}

		if (rAddress != 0)
			FastPatch::make(rAddress, hexColor.substr(0, 2));

		if (gAddress != 0)
			FastPatch::make(gAddress, hexColor.substr(2, 2));

		if (bAddress != 0)
			FastPatch::make(bAddress, hexColor.substr(4, 2));

		if (aAddress != 0)
			FastPatch::make(aAddress, hexColor.substr(6, 2));
	}
};

class AlertColorCustomizerManager {
	enum DataLoadingResult {
		OK,
		FileNotFound,
		ParsingError
	};

	std::vector<ColorPatch*> colorPatches;

	DataLoadingResult loadingStatus;
	static AlertColorCustomizerManager* instance;

	void init() {
		loadingStatus = loadData();
		for (auto patch : colorPatches) {
			patch->apply();
		}
	}

	DataLoadingResult loadData() {
		std::ifstream file("Resources/alertColorCustomizer.json");
		if (!file) return FileNotFound;
		std::ostringstream buffer;
		buffer << file.rdbuf();
		std::string fileContent = buffer.str();

		file.close();
		try {
			auto root = nlohmann::json::parse(fileContent);

			if (!root.is_object()) return ParsingError;

			for (const auto& [name, colorPatch] : root.items()) {
				if (!colorPatch.is_object() || !colorPatch.contains("color") || !colorPatch["color"].is_string() || !colorPatch.contains("address") || !colorPatch["address"].is_object()) return ParsingError;

				std::string color = colorPatch["color"];

				if (colorPatch["address"].contains("all")) {
					auto allOpt = FastPatch::hexStringToAddress(colorPatch["address"]["all"]);
					if (!allOpt.has_value()) continue;
					auto newColorPatch = new ColorPatch(color, allOpt.value());
					colorPatches.push_back(newColorPatch);
				}
				else if (colorPatch["address"].contains("r") || colorPatch["address"].contains("g") || colorPatch["address"].contains("b") || colorPatch["address"].contains("a")) {
					auto rOpt = colorPatch["address"].contains("r") ? FastPatch::hexStringToAddress(colorPatch["address"]["r"]) : std::nullopt;
					auto gOpt = colorPatch["address"].contains("g") ? FastPatch::hexStringToAddress(colorPatch["address"]["g"]) : std::nullopt;
					auto bOpt = colorPatch["address"].contains("b") ? FastPatch::hexStringToAddress(colorPatch["address"]["b"]) : std::nullopt;
					auto aOpt = colorPatch["address"].contains("a") ? FastPatch::hexStringToAddress(colorPatch["address"]["a"]) : std::nullopt;

					auto newColorPatch = new ColorPatch(color, rOpt.value_or(0), gOpt.value_or(0), bOpt.value_or(0), aOpt.value_or(0));
					colorPatches.push_back(newColorPatch);
				}
				else {
					std::cout << "empty colorPatch" << std::endl;
				}
			}
		}
		catch (const std::exception& e) {
			return ParsingError;
		}
		return OK;
	}

	void createErrorLabel(CCLayer* layer) {
		std::string errorText;
		switch (loadingStatus) {
		case AlertColorCustomizerManager::FileNotFound:
			errorText = "Can't find 'alertColorCustomizer.json' in ./Resources";
			break;
		case AlertColorCustomizerManager::ParsingError:
			errorText = "Can't parse 'alertColorCustomizer.json'";
			break;
		}

		auto size = CCDirector::sharedDirector()->getWinSize();

		auto errorLabel = CCLabelBMFont::create(errorText.c_str(), "bigFont.fnt");
		errorLabel->setColor({ 255, 0, 0 });
		errorLabel->setScale(0.6);
		errorLabel->setPosition({ size.width / 2, size.height - 10 });
		layer->addChild(errorLabel);
	}

	AlertColorCustomizerManager() {};
public:

	void onMenuLayer(CCLayer* layer) {
		if (loadingStatus != OK) {
			createErrorLabel(layer);
			return;
		}
	}

	static AlertColorCustomizerManager* getInstance() {
		if (!instance) {
			instance = new AlertColorCustomizerManager();
			instance->init();
		}
		return instance;
	}

};
AlertColorCustomizerManager* AlertColorCustomizerManager::instance = nullptr;

bool(__thiscall* MenuLayer_init)(MenuLayer* self);
bool __fastcall MenuLayer_init_H(MenuLayer* self, void*) {
	if (!MenuLayer_init(self)) return false;
	AlertColorCustomizerManager::getInstance()->onMenuLayer(self);
	return true;
}

void inject() {
#if _WIN32
	auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

	MH_CreateHook(
		reinterpret_cast<void*>(base + 0x1907b0),
		reinterpret_cast<void*>(&MenuLayer_init_H),
		reinterpret_cast<void**>(&MenuLayer_init)
	);

	MH_EnableHook(MH_ALL_HOOKS);
#endif
}

#if _WIN32
WIN32CAC_ENTRY(inject)
#endif
	