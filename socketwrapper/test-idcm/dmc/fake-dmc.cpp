#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>
#include <assert.h>
#include <ctype.h>      // for toupper
#include "interface/msg-passer-public.h"
#include "fmt/format.h"

class DMC : public IVehicleNotification
{
public:
    void ToCloud(const std::string& notification) override;
    void ToHMI(const UpgradeStatus& status) override;
};

void DMC::ToCloud(const std::string& notification)
{
    std::cout << notification << std::endl;
}

void DMC::ToHMI(const UpgradeStatus& status)
{
    std::cout << "Upgrade status: " << std::endl;
    std::cout << "\t" << status.dev_id << std::endl;
    std::cout << "\t" << status.soft_id << std::endl;
    std::cout << "\t" << status.door_module << std::endl;
    std::cout << "\t" << status.esti_time << std::endl;
    std::cout << "\t" << status.start_time << std::endl;
    std::cout << "\t" << status.time_stamp << std::endl;
    std::cout << "\t" << status.status << std::endl;
    std::cout << "\t" << status.progress_percent << std::endl;    
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << argv[0] << " L1-Manifest" <<std::endl;
        return  -1;
    }

    std::ifstream json_file{argv[1]};
    std::string json((std::istreambuf_iterator<char>(json_file)),
                    std::istreambuf_iterator<char>());

    IMessagePasser* passer = GetMessagePasser();
    auto callback = std::make_shared<DMC>();

    if (passer->Init(callback)) {
        passer->ToVehicle(json);
    }

    std::string                 service_pack_id;
    std::vector<ReleaseNote>    release_notes;
    while(true) {
        if (passer->CheckNewPackage(service_pack_id, release_notes)) {
            std::cout << "New package: " << service_pack_id << std::endl;
            std::cout << "Release notes: " << std::endl;

            for (auto& note : release_notes) {
                std::cout << note.locale << std::endl;
                std::cout << note.description << std::endl;
            }

            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << "Do you want to upgrade ? (Y/N)" << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(2));

            if (passer->StartUpgrade(service_pack_id)) {
                std::cout << fmt::format("Upgrading {} ...", service_pack_id) << std::endl;
            } else {
                std::cout << fmt::format("Fail to upgrade {}", service_pack_id) << std::endl;
                return  -2;
            }
            break;
        } else {
            // wait for 1 seconds
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::cout << "Wait upgrading message ..." << std::endl;
    std::cout << "Press any key to exit" << std::endl;
    std::cin.get();

    return  0;
}