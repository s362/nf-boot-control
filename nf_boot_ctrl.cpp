#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/vtable.hpp>
#include <variant>

#define MAX_NF_CARD_NUMS 16

#define GPIO_IN   0
#define GPIO_OUT  1

namespace nf_boot_ctrl
{
  static boost::asio::io_service io;
  static std::shared_ptr<sdbusplus::asio::connection> conn;
  static std::shared_ptr<sdbusplus::asio::dbus_interface> nfBladeIface[MAX_NF_CARD_NUMS];

  static constexpr const char* nfBootService = "xyz.openbmc_project.nf.boot.manager";
  static constexpr const char* nfBootIface = "xyz.openbmc_project.NF.Blade.Boot";
  static std::string nfBootPath = "/xyz/openbmc_project/control/nf/";
  static std::string nfBladePath[MAX_NF_CARD_NUMS];

int main(int argc, char* argv[])
{
  std::cerr << "Start NF card boot control service...\n";
  nf_boot_ctrl::conn = 
  std::make_shared<sdbusplus::asio::connection>(nf_boot_ctrl::io);
  
  nf_boot_ctrl::conn->request_name(nf_boot_ctrl::nfBootService);
  
  sdbusplus::asio::object_server server =
  sdbusplus::asio::object_server(nf_boot_ctrl::conn);

  std::string gpio_name;
  int i;
  
  for (i = 0; i < MAX_NF_CARD_NUMS; i++) {
    std::string current_blade;
    
    /** set nf blade dbus path */
    nf_boot_ctrl::nfBladePath[i] = 
    nf_boot_ctrl::nfBootPath + "blade" + std::to_string(i);

    current_blade.assign(nf_boot_ctrl::nfBladePath[i].c_str());
    
    /** setup nf/blade<x> dbus object */
    nf_boot_ctrl::nfBladeIface[i] = 
    server.add_interface(
      nf_boot_ctrl::nfBladePath[i], nf_boot_ctrl::nfBootIface);
    
    /** add *BootMode * dbus property to nf/blade<x>/ dbus object */
    nf_boot_ctrl::nfBladeIface[i]->register_property("BootMode", 
      std::string("SD"), 
      sdbusplus::asio::PropertyPermission::readWrite);
    
    /** add *Reset* dbus property to nf/blade<x>/ dbus object */
    nf_boot_ctrl::nfBladeIface[i]->register_property("OneTime", 
      std::string("true"), 
      sdbusplus::asio::PropertyPermission::readWrite);
    
    nf_boot_ctrl::nfBladeIface[i]->initialize();
  }
  
  nf_boot_ctrl::io.run();
  
  return 0;
}