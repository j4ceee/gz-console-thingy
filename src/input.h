#pragma once

#include <Windows.h>
#include <cstdint>

struct ID3D11DeviceContext;

#include "singleton.h"

class Input : public Singleton<Input>
{
  public:
    Input()          = default;
    virtual ~Input() = default;

    void Draw(ID3D11DeviceContext* context);
    bool FeedEvent(uint32_t message, WPARAM wParam, LPARAM lParam);
};
