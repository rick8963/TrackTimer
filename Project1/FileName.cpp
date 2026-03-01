#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <iomanip>
#include "Track.h"
#include "GPSPoint.h"

using namespace std;

using TimeMs = uint32_t;

std::vector<std::string> splitNmea(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        fields.push_back(item);
    }
    return fields;
}

// Parse NMEA time hhmmss.sss into integer milliseconds
TimeMs parseNmeaTime(const std::string& timeStr) {
    if (timeStr.length() < 6) return 0;

    uint32_t hours = static_cast<uint32_t>(std::stoi(timeStr.substr(0, 2)));
    uint32_t minutes = static_cast<uint32_t>(std::stoi(timeStr.substr(2, 2)));
    float seconds_f = std::stof(timeStr.substr(4));
    uint32_t seconds = static_cast<uint32_t>(seconds_f);
    uint32_t millis = static_cast<uint32_t>((seconds_f - seconds) * 1000.0f + 0.5f);

    return hours * 3600000u + minutes * 60000u + seconds * 1000u + millis;
}

// Parse GPRMC with timestamp
bool parseGPRMC_raw(const std::string& line, double& lat_raw, double& lon_raw,
    char& lat_hemi, char& lon_hemi, TimeMs& timestamp) {
    auto f = splitNmea(line);
    if (f.size() < 7) return false;
    if (f[0].find("GPRMC") == std::string::npos) return false;
    if (f[2] != "A") return false;

    timestamp = parseNmeaTime(f[1]);
    lat_raw = std::atof(f[3].c_str());
    lat_hemi = f[4].empty() ? 'N' : f[4][0];
    lon_raw = std::atof(f[5].c_str());
    lon_hemi = f[6].empty() ? 'E' : f[6][0];

    return true;
}

// Parse GPGGA with timestamp
bool parseGPGGA_raw(const std::string& line, double& lat_raw, double& lon_raw,
    char& lat_hemi, char& lon_hemi, TimeMs& timestamp) {
    auto f = splitNmea(line);
    if (f.size() < 6) return false;
    if (f[0].find("GPGGA") == std::string::npos) return false;

    timestamp = parseNmeaTime(f[1]);
    lat_raw = std::atof(f[2].c_str());
    lat_hemi = f[3].empty() ? 'N' : f[3][0];
    lon_raw = std::atof(f[4].c_str());
    lon_hemi = f[5].empty() ? 'E' : f[5][0];

    return true;
}

int main() {
    vector<Line2D> TKSsectors;
    TKSsectors.push_back(Line2D(GPSPoint(22.742248, 120.322181, true), 0, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.742798, 120.321496, true), 180, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.742724, 120.322010, true), 180, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.742285, 120.321387, true), 60, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.742540, 120.321959, true), 88, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.741863, 120.321912, true), 262, 20));
    TKSsectors.push_back(Line2D(GPSPoint(22.741763, 120.321930, true), 81, 20));
    Track TKS(TKSsectors);

    std::ifstream file("TKS.nmea");
    if (!file.is_open()) {
        std::cerr << "Unable to open TKS.nmea\n";
        return 1;
    }

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        if (line.empty() || line[0] != '$') continue;

        double lat_raw, lon_raw;
        char lat_hemi, lon_hemi;
        TimeMs timestamp;

        if (parseGPRMC_raw(line, lat_raw, lon_raw, lat_hemi, lon_hemi, timestamp)) {
            GPSPoint currentPos(lat_raw, lon_raw);
            TKS.updatePos(currentPos, timestamp);

            Line2D curSector = TKS.getNextCheckpoint();
            const vector<Sector>& sectors = TKS.getSectors();

            /*cout << "current pos (" << currentPos.getX() << ", " << currentPos.getY()
                << ") at sector " << TKS.getCurrentSectorCount() + 1
                << "\n" << curSector.distanceToLine(currentPos)
                << " meters to next checkpoint ("
                << curSector.getPoint1().getX() << ", " << curSector.getPoint1().getY()
                << "), ("
                << curSector.getPoint2().getX() << ", " << curSector.getPoint2().getY()
                << ")\n";*/

            //cout << "\n=== Sector Status ===\n";
            //for (const auto& sector : TKS.getSectors()) {
            //    cout << "Sector " << sector.getStartNodeIndex() + 1 << ": "
            //        << (sector.isPassed() ? " Passed" : " Not passed") << "\n";
            //}
        }
        //Sleep(500);
    }

    file.close();
    cout << fixed << setprecision(3);
    cout << TKS.getLaps().size() << " laps in total\n";
    int i = 1;
    for (auto& lap : TKS.getLaps()) {
        cout << "Lap " << i++ << " : " << lap.getLapTime() / 1000.0 << " seconds\n";
        for (int j = 0; j < TKS.getSectorCount(); j++) {
            auto sectorTime = lap.getSectorTime(j);
            if (sectorTime.has_value()) {
                cout << "\tSector " << j + 1 << " : " << sectorTime.value() / 1000.0 << " seconds\n";
            }
            else {
                cout << "\tSector " << j + 1 << " : N/A\n";
            }
        }
    }

    cout << "\n=== Session Statistics ===\n";
    cout << "Best lap time: " << TKS.getBestLapTime() / 1000.0 << " seconds\n";
    cout << "Latest lap time: " << TKS.getLatestLapTime() / 1000.0 << " seconds\n";

    return 0;
}