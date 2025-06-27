#pragma once
#include "OnlyForTestScene.h"
#include "Camera2D.h"
#include <list>
//#include <vector>
#include <wrl/client.h>
#include <d2d1_1.h>

constexpr int  MAX_SIZE = 8;

class BoxObject;

class TransformPracticeScene : public OnlyForTestScene
{
public:
    TransformPracticeScene() = default;
    virtual ~TransformPracticeScene();
    
    void SetUp(HWND hWnd) override;

    void Tick(float deltaTime) override;

    void OnResize(int width, int height) override;

private:

    void ProcessKeyboardEvents();

    void OnMouseLButtonDown(D2D1_POINT_2F point) override;

    void OnMouseRButtonDown(D2D1_POINT_2F point) override;

    void AddBoxObjects(D2D1_POINT_2F point, int i_image);

    void ClearBoxObjects();

    void SelectBoxObject(D2D1_POINT_2F point);

    void SetSelfRotation();

    void UpdateRelationship();
   
    UnityCamera m_UnityCamera;

    ComPtr<ID2D1Bitmap1> m_BitmapPtr[MAX_SIZE];

    std::vector<BoxObject*> m_BoxObjects;

    std::list<BoxObject*> m_SelectedBoxObjects;

    int m_objCnt = 0;
};

