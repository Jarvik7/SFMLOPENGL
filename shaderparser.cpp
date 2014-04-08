class q3shader {
public:
	int cull = 1; // 0 = None, 1 = Front (default), 2 = back
	void parse(std::vector<std::string> token) {
		if (tokens[0].at(0) == '/') continue; // Comment
		if (tokens[0] == "surfaceparm") continue; // Not needed yet. May eventually need for collision detection
		if (tokens[0] == "q3map_surfacelight") continue; // Not needed yet. May eventually need for global illumination
		if (tokens[0] == "q3map_lightsubdivide") continue; // Not needed yet. May eventually need for global illumination
		if (tokens[0] == "q3map_globaltexture") continue; // Not needed?
		if (tokens[0] == "cull") {
			if (tokens.size() == 1 || tokens[1] == "front") { cull = 1; continue; }
			if (tokens[1] == "disable" || tokens[1] == "none") { cull = 0; continue; }
			if (tokens[1] == "back") { cull = 2; continue; }
			else std::cout << "Unknown cull parameters.\n";
		}
	}
};