// Copyright (C) 2019 Yu Yang
// 
// This file is part of Vesyla.
// 
// Vesyla is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Vesyla is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Vesyla.  If not, see <http://www.gnu.org/licenses/>.

#include "VDump.hpp"
#include <fstream>
#include "util/bitmap/bitmap_image.hpp"

void VDump::dump_layout(vector<vector<int>> layout)
{
	vesyla::util::GlobalVar glv;
	string path;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	path = path + "layout/";
	mkdir(path.c_str(), 0755);

	string input_file_name;
	CHECK(glv.gets("input_file_name", input_file_name));

	string path_csv = path + input_file_name + ".csv";
	std::ofstream os(path_csv, std::ofstream::out | std::ofstream::trunc);
	os << "# start_row | start_col | height | width | cell_type\n";
	for (int i = 0; i < layout.size(); i++)
	{
		os << to_string(layout[i][0]) << ","
			 << to_string(layout[i][1]) << ","
			 << to_string(layout[i][2]) << ","
			 << to_string(layout[i][3]) << ","
			 << to_string(layout[i][4]) << "\n";
	}
	os.close();

	int ZOOM_FACTOR = 20;
	int H = 0;
	int W = 0;
	for (unsigned j = 0; j < layout.size(); j++)
	{
		if (H < layout[j][0] + layout[j][2])
		{
			H = layout[j][0] + layout[j][2];
		}
		if (W < layout[j][1] + layout[j][3])
		{
			W = layout[j][1] + layout[j][3];
		}
	}

	bitmap_image image(W * ZOOM_FACTOR + 1, H * ZOOM_FACTOR + 1);
	image_drawer draw(image);
	const rgb_t colormap[] = {
			{228, 26, 28},
			{55, 126, 184},
			{77, 175, 74},
			{152, 78, 163},
			{255, 127, 0},
			{255, 255, 51},
			{166, 86, 40},
			{247, 129, 191},
			{153, 153, 153}};
	for (unsigned j = 0; j < layout.size(); j++)
	{
		draw.pen_width(2);
		draw.pen_color(255, 255, 255);
		draw.rectangle(layout[j][1] * ZOOM_FACTOR + 1, layout[j][0] * ZOOM_FACTOR + 1, (layout[j][1] + layout[j][3]) * ZOOM_FACTOR - 2, (layout[j][0] + layout[j][2]) * ZOOM_FACTOR - 2);
		draw.pen_width(1);
		draw.pen_color(colormap[layout[j][4]].red, colormap[layout[j][4]].green, colormap[layout[j][4]].blue);
		draw.fill_rectangle(layout[j][1] * ZOOM_FACTOR + 3, layout[j][0] * ZOOM_FACTOR + 3, (layout[j][1] + layout[j][3]) * ZOOM_FACTOR - 3, (layout[j][0] + layout[j][2]) * ZOOM_FACTOR - 3);
	}
	for (unsigned j = 0; j < ceil(image.width() / float(ZOOM_FACTOR)); j++)
	{
		draw.pen_color(20, 20, 20);
		draw.vertical_line_segment(0, image.height(), j * ZOOM_FACTOR);
	}
	for (unsigned j = 0; j < ceil(image.height() / float(ZOOM_FACTOR)); j++)
	{
		draw.pen_color(20, 20, 20);
		draw.horizontal_line_segment(0, image.width(), j * ZOOM_FACTOR);
	}
	string path_bmp = path + input_file_name + ".bmp";
	image.save_image(path_bmp.c_str());
}
