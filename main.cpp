#include "web_server/http_web_server.h"


int main(int argc, char*argv[]) 
{
    HTTPWebServer app;
    std::cout << "Started..." << std::endl;
    return app.run(argc, argv);
}