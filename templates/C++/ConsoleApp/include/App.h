#ifndef APP_H
#define APP_H

#include "../dependencies/CanvasApplication"

class App : public CanvasApplication
{
public:
    App();
    ~App();

    void Run() override;
    void Init() override;
    void Update() override;
    void Draw() const override;
private:

};

#endif // APP_H