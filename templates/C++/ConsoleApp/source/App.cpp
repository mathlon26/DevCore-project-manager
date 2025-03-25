#include "../include/App.h"

App::App()
{
}

App::~App()
{
}

// Reset the entire App (makes multiple runs possible)
void App::Init()
{
    Draw();
}

// The app is single threaded
// Draw will wait on Update
void App::Run()
{
    Init();

    while (ShouldRun())
    {
        Update();
        Draw();
    }
}

// Called every cycle
void App::Update()
{
}

// Called every cycle
void App::Draw() const
{
}