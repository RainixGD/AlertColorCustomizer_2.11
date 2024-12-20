#include "./includes.h"
#include "./ColorRGBA.h"
#include "./FastPatch.h"

std::array<int, 11> backgroundsAdresses = { 0x15C02C, 0x15926F, 0x9C874, 0x9D6A4, 0x1523AC, 0x17D5EC, 0x193E61, 0x20F447, 0x2138A1, 0x25E131, 0x113656};
std::array<std::array<int, 4>, 6> Scale9SpritesAdresses = { {
	{ 0x17DFF6, 0x17DFF7, 0x17DFF9, 0x17DF74 },
	{ 0x17DF05, 0x17DF06, 0x17DF08, 0x17DEB1 },
	{ 0x17E419, 0x17E41A, 0x17E413, 0x17E3BC },
	{ 0x17E939, 0x17E93A, 0x17E93C, 0x17E8DE },
	{ 0x17EC15, 0x17EC16, 0x17EC18, 0x17EBBA },
	{ 0x14FE01, 0x14FE02, 0x14FE0B, 0x14FE0B },
} };

class AlertColorCustomizer {

	enum DataLoadingResult {
		OK,
		FileNotFound,
		ParsingError
	};

	std::unordered_map<std::string, ColorRGBA*> backgrounds;
	std::unordered_map<std::string, std::pair<ColorRGBA*, ColorRGBA*>> cells;
	std::unordered_map<std::string, ColorRGBA*> scale9Sprites;

	DataLoadingResult loadingStatus;
	static AlertColorCustomizer* instance;

	void init() {
		loadingStatus = loadData();
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

			if (!root.contains("backgrounds") || !root["backgrounds"].is_object()
				|| !root.contains("cells") || !root["cells"].is_object()
				|| !root.contains("Scale9Sprites") || !root["Scale9Sprites"].is_object()) return ParsingError;

			for (const auto& [key, value] : root["backgrounds"].items()) {
				if (!value.is_string()) return ParsingError;

				auto color = ColorRGBA::create(value.get<std::string>());
				if (!color) continue;

				backgrounds[key] = color;
			}

			for (const auto& [key, value] : root["cells"].items()) {
				if (!value.is_object() || !value.contains("color") || !value.contains("color2")) return ParsingError;

				auto color = ColorRGBA::create(value["color"].get<std::string>());
				auto color2 = ColorRGBA::create(value["color2"].get<std::string>());
				if (!color || !color2) continue;

				cells[key] = std::make_pair(color, color2);
			}

			for (const auto& [key, value] : root["scale9Sprites"].items()) {
				if (!value.is_string()) return ParsingError;

				auto color = ColorRGBA::create(value.get<std::string>());
				if (!color) continue;

				scale9Sprites[key] = color;
			}
		}
		catch (...) {
			return ParsingError;
		}
		return OK;
	}

	void createErrorLabel(CCLayer* layer) {
		std::string errorText;
		switch (loadingStatus) {
		case AlertColorCustomizer::FileNotFound:
			errorText = "Can't find 'alertColorCustomizer.json' in ./Resources";
			break;
		case AlertColorCustomizer::ParsingError:
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

	AlertColorCustomizer() {};
public:

	static AlertColorCustomizer* getInstance() {
		if (!instance) {
			instance = new AlertColorCustomizer();
			instance->init();
		}
		return instance;
	}

};
AlertColorCustomizer* AlertColorCustomizer::instance = nullptr;


void(__thiscall* LevelBrowserLayer_setupLevelBrowser)(LevelBrowserLayer* self, cocos2d::CCArray* levels);
void __fastcall LevelBrowserLayer_setupLevelBrowser_H(LevelBrowserLayer* self, void*, cocos2d::CCArray* levels) {
	LevelBrowserLayer_setupLevelBrowser(self, levels);
	std::cout << "aboba" << std::endl;
	self->m_list->setColor(ccc3(0, 100, 100));
}



void inject() {
#if _WIN32
	auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

	AllocConsole();
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	
	FastPatch::make("0x17DFF6", "FF FF");
	FastPatch::make("0x17DFF9", "FF");


	MH_CreateHook(
		reinterpret_cast<void*>(base + 0x15bb40),
		reinterpret_cast<void*>(&LevelBrowserLayer_setupLevelBrowser_H),
		reinterpret_cast<void**>(&LevelBrowserLayer_setupLevelBrowser)
	);

	MH_EnableHook(MH_ALL_HOOKS);
#endif
}

#if _WIN32
WIN32CAC_ENTRY(inject)
#endif
/*
	__declspec(dllexport) static void updateBGCell(LevelCell* cell, unsigned int index) {
		AlertColorCustomizer::updateCell(cell, index, 0);
	}
	__declspec(dllexport) static void updateBGStatsCell(LevelCell* cell, unsigned int index) {
		AlertColorCustomizer::updateCell(cell, index, 1);
	}
	__declspec(dllexport) static void updateBGCommentCell(LevelCell* cell, unsigned int index) {
		AlertColorCustomizer::updateCell(cell, index, 2);
	}
	__declspec(dllexport) static void updateBGSongCell(LevelCell* cell, unsigned int index) {
		AlertColorCustomizer::updateCell(cell, index, 3);
	}
	static void updateCell(LevelCell* cell, unsigned int index, int type) {
		if (!Colors.cellsColors[type].isActive) return;
		if (index % 2) {
			cell->m_pBGLayer->setColor({ Colors.cellsColors[type].r, Colors.cellsColors[type].g, Colors.cellsColors[type].b });
			cell->m_pBGLayer->setOpacity(Colors.cellsColors[type].a);
		}
		else {
			cell->m_pBGLayer->setColor({ Colors.cellsColors[type].r2, Colors.cellsColors[type].g2, Colors.cellsColors[type].b2 });
			cell->m_pBGLayer->setOpacity(Colors.cellsColors[type].a2);
		}
	}
	static int getColorByte(int byteNum, std::string colorString) {
		std::string byteString = colorString.substr(byteNum * 2 - 2, 2);
		int byte = std::stoi(byteString, nullptr, 16);
		return byte;
	}
	static void patchBackground(int index) {
		if (!Colors.backgroundsColors[index].isActive) return;
		void* address = reinterpret_cast<void*>(gd::base + backgroundsAdresses[index]);
		unsigned char bytes[] = { 
			static_cast<unsigned char>(Colors.backgroundsColors[index].r),
			static_cast<unsigned char>(Colors.backgroundsColors[index].g),
			static_cast<unsigned char>(Colors.backgroundsColors[index].b),
			static_cast<unsigned char>(Colors.backgroundsColors[index].a)
		};
		WriteProcessMemory(GetCurrentProcess(), address, bytes, sizeof(bytes), nullptr);
	}
	static void patchScale9Sprite(int index) {
		if (!Colors.Scale9SpritesColors[index].isActive) return;
		unsigned char r[] = { static_cast<unsigned char>(Colors.Scale9SpritesColors[index].r) };
		unsigned char g[] = { static_cast<unsigned char>(Colors.Scale9SpritesColors[index].g) };
		unsigned char b[] = { static_cast<unsigned char>(Colors.Scale9SpritesColors[index].b) };
		unsigned char a[] = { static_cast<unsigned char>(Colors.Scale9SpritesColors[index].a) };
		WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + Scale9SpritesAdresses[index][0]), r, sizeof(r), nullptr);
		WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + Scale9SpritesAdresses[index][1]), g, sizeof(g), nullptr);
		WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + Scale9SpritesAdresses[index][2]), b, sizeof(b), nullptr);
		WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + Scale9SpritesAdresses[index][3]), a, sizeof(a), nullptr);
	}*/
