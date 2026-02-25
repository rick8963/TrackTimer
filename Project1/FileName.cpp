#include<iostream>
#include<vector>
#include <sstream>
#include <fstream>
#include<Windows.h>
#include"Track.h"
#include"GPSPoint.h"

using namespace std;

// dummy funcion to update postion

std::vector<std::string> splitNmea(const std::string& line)
{
	std::vector<std::string> fields;
	std::stringstream ss(line);
	std::string item;
	while (std::getline(ss, item, ',')) {
		fields.push_back(item);
	}
	return fields;
}

// 解析 GPRMC：取得 ddmm.mmmm / dddmm.mmmm 形式的原始值
bool parseGPRMC_raw(const std::string& line, double& lat_raw, double& lon_raw,
	char& lat_hemi, char& lon_hemi)
{
	auto f = splitNmea(line);
	if (f.size() < 7) return false;
	if (f[0].find("GPRMC") == std::string::npos) return false;
	if (f[2] != "A") return false; // A = 有效定位

	lat_raw = std::atof(f[3].c_str());   // 例如 2244.49650
	lat_hemi = f[4].empty() ? 'N' : f[4][0]; // N / S
	lon_raw = std::atof(f[5].c_str());   // 例如 12019.30793
	lon_hemi = f[6].empty() ? 'E' : f[6][0]; // E / W

	return true;
}

// 解析 GPGGA：取得 ddmm.mmmm / dddmm.mmmm 形式的原始值
bool parseGPGGA_raw(const std::string& line, double& lat_raw, double& lon_raw,
	char& lat_hemi, char& lon_hemi)
{
	auto f = splitNmea(line);
	if (f.size() < 6) return false;
	if (f[0].find("GPGGA") == std::string::npos) return false;

	lat_raw = std::atof(f[2].c_str());
	lat_hemi = f[3].empty() ? 'N' : f[3][0];
	lon_raw = std::atof(f[4].c_str());
	lon_hemi = f[5].empty() ? 'E' : f[5][0];

	return true;
}

int main()
{
	vector<Line2D> TKSsectors;
	TKSsectors.push_back(Line2D(GPSPoint(2244.515, 12019.328), 5, 30));
	TKSsectors.push_back(Line2D(GPSPoint(2244.572, 12019.309), 270, 30));
	TKSsectors.push_back(Line2D(GPSPoint(2244.550, 12019.297), 90, 30));
	TKSsectors.push_back(Line2D(GPSPoint(2244.510, 12019.314), 255, 30));
	Track TKS(TKSsectors);


	std::ifstream file("TKS.nmea");  // 改成你的檔案名
	if (!file.is_open()) {
		std::cerr << "無法開啟 TKS.nmea\n";
		return 1;
	}

	std::string line;
	int line_num = 0;
	while (std::getline(file, line)) {
		line_num++;
		if (line.empty() || line[0] != '$') continue;

		double lat_raw, lon_raw;
		char lat_hemi, lon_hemi;


		if (parseGPRMC_raw(line, lat_raw, lon_raw, lat_hemi, lon_hemi)) {};

		/*double lat_dec = (int(lat_raw / 100) + (lat_raw - int(lat_raw / 100) * 100) / 60.0);
		double lon_dec = (int(lon_raw / 100) + (lon_raw - int(lon_raw / 100) * 100) / 60.0);*/
		//printf("Lat: %.5f\tLon: %.5f\n", lat_dec, lon_dec);  // Lat: N 2244.49650
		
		GPSPoint currentPos(lat_raw, lon_raw);
		TKS.updatePos(currentPos);

		Line2D curSector = TKS.getNextCheckpoint();
		const vector<Sector>& sectors = TKS.getSectors();

		cout << "current pos (" << currentPos.getX() << ", " << currentPos.getY() << ")\tat sector " << TKS.getCurrentSectorCount() + 1 << endl
			<< curSector.distanceToLine(currentPos) << " meters to next checkpoint (" << curSector.getPoint1().getX() << ", " << curSector.getPoint1().getY() << "), "
			<< "(" << curSector.getPoint2().getX() << ", " << curSector.getPoint2().getY() << ")" << endl;
		cout << "\n=== Sector Status ===" << endl;
		for (const auto& sector : TKS.getSectors())
		{
			cout << "Sector " << sector.getStartNodeIndex() + 1 << ": "
				<< (sector.isPassed() ? " Passed" : " Not passed") << endl;
		}
		//Sleep(500);
	}

	file.close();
	
	cout << TKS.getLaps().size() << " laps in total" << endl;
	int i = 1;
	for (auto& lap : TKS.getLaps())
	{
		cout << "Lap " << i++ << " : " << lap.getLapTime() << " ticks" << endl;
		for (int j = 0; j < TKS.getSectorCount() ; j++)
		{
			auto sectorTime = lap.getSectorTime(j);
			if (sectorTime.has_value())
				cout << "\tSector " << j + 1 << " : " << sectorTime.value() << " ticks" << endl;
			else
				cout << "\tSector " << j + 1 << " : " << "N/A" << endl;
		}
			
	}

	cout << "\n=== Session Statistics ===" << endl;
	cout << "Best lap time: " << TKS.getBestLapTime() << " ticks" << endl;
	cout << "Latest lap time: " << TKS.getLatestLapTime() << " ticks" << endl;

	 //22.857318, 120.289120
	 //22.856466, 120.289463

	//GPSPoint loc(2251.4390, 12017.3472);
	//cout << loc.getX() << " " << loc.getY() << endl;
	//GPSPoint loca(2251.3879, 12017.3677);
	//cout << loca.getX() << " " << loca.getY() << endl;
	//cout << "distance " << loc.distanceTo(loca) << " meters" << endl;

	//std::string rmc = "$GPRMC,072402.700,A,2244.49650,N,12019.30793,E,0.00,290.70,230126,,,*28";

	//double lat_raw, lon_raw;
	//char lat_hemi, lon_hemi;

	//if (parseGPRMC_raw(rmc, lat_raw, lon_raw, lat_hemi, lon_hemi)) {
	//	printf("Lat: %c %.5f\n", lat_hemi, lat_raw);  // Lat: N 2244.49650
	//	printf("Lon: %c %.5f\n", lon_hemi, lon_raw);  // Lon: E 12019.30793
	//}
	return 0;
}