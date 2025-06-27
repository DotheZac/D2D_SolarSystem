#include "pch.h"
#include "InputManager.h"
#include "D2DTransform.h"
#include "SolarSystemRenderer.h"
#include "TransformPracticeScene.h"

using TestRenderer = myspace::D2DRenderer;
using Vec2 = MYHelper::Vector2F;

class BoxObject
{
    BoxObject() = delete;
    BoxObject(const BoxObject&) = delete;
    void operator=(const BoxObject&) = delete;

public:
    BoxObject(ComPtr<ID2D1Bitmap1>& bitmap)
    {
        m_BitmapPtr = bitmap;

        ++s_id;
        m_name += std::to_wstring(s_id); //ID�� �̸��� �߰�

        m_renderTM = MYTM::MakeRenderMatrix(true);

        D2D1_SIZE_F size = { m_rect.right - m_rect.left, m_rect.bottom - m_rect.top };

        //m_transform.SetPivotPreset(D2DTM::PivotPreset::TopLeft, size);
        //m_transform.SetPivotPreset(D2DTM::PivotPreset::BottomRight, size);
        //m_transform.SetPivotPreset(D2DTM::PivotPreset::Center, size);

    }

    ~BoxObject() = default;

    void Update(float deltaTime)
    {
        if (m_isSelfRotation)
        {
            m_transform.Rotate(deltaTime * 36.f); // �ڱ� ȸ��
        }
    }

    void Draw(TestRenderer& testRender, D2D1::Matrix3x2F viewTM)
    {
        static D2D1_RECT_F s_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f);

        D2D1::Matrix3x2F worldTM = m_transform.GetWorldMatrix();
        D2D1::Matrix3x2F finalTM = m_renderTM * worldTM * viewTM;
        D2D1::ColorF boxColor = D2D1::ColorF::LightGray;

        if (m_isLeader) boxColor = D2D1::ColorF::Red;
        else if (m_isSelected) boxColor = D2D1::ColorF::HotPink;

        testRender.SetTransform(finalTM);
        testRender.DrawRectangle(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom, boxColor);

        D2D1_RECT_F dest = D2D1::RectF(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom);

        testRender.DrawBitmap(m_BitmapPtr.Get(), dest);
        testRender.DrawMessage(m_name.c_str(), s_rect.left, s_rect.top, 200, 50, D2D1::ColorF::Black);

    }


    void SetPosition(const Vec2& pos)
    {
        m_transform.SetPosition(pos);
    }

    void Move(const Vec2& offset)
    {
        m_transform.Translate(offset);
    }

    void Rotate(float angle)
    {
        m_transform.Rotate(angle);
    }

    void ToggleSelected()
    {
        m_isSelected = !m_isSelected;
    }

    bool IsSelected() const
    {
        return m_isSelected;
    }

    void ToggleSelfRotation()
    {
        m_isSelfRotation = !m_isSelfRotation;
    }

    bool IsHitTest(D2D1_POINT_2F worldPoint, D2D1::Matrix3x2F viewTM)
    {
        D2D1::Matrix3x2F worldTM = m_transform.GetWorldMatrix();

        D2D1::Matrix3x2F finalTM = m_renderTM * worldTM * viewTM;

        finalTM.Invert();

        // 2) ���� ��ǥ�� ����Ʈ ��ȯ
        D2D1_POINT_2F localPt = finalTM.TransformPoint(worldPoint);

        // 3) ���� �簢�� ����
        // (0,0) ~ (width, height) ������ �ִٸ� ��Ʈ!
        // m_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f);

        std::cout << "BoxObject::IsHitTest: localPt = ("
            << localPt.x << ", " << localPt.y << ")" << std::endl;

        std::cout << "BoxObject::IsHitTest: m_rect = ("
            << m_rect.left << ", " << m_rect.top << ", "
            << m_rect.right << ", " << m_rect.bottom << ")" << std::endl;

        // 4) ���� �������� �˻�
        return MYTM::IsPointInRect(localPt, m_rect);
    }

    D2DTM::Transform* GetTransform()
    {
        return &m_transform;
    }

    void SetParent(BoxObject* parent)
    {
        assert(parent != nullptr);

        if (nullptr != m_transform.GetParent())
        {
            // �̹� �θ� �ִٸ� �θ� ���踦 �����մϴ�.
            m_transform.DetachFromParent();
        }

        m_transform.SetParent(parent->GetTransform());
    }

    void DetachFromParent()
    {
        m_transform.DetachFromParent();
    }

    void SetLeader(bool isLeader)
    {
        m_isLeader = isLeader;
    }

private:
    D2DTM::Transform m_transform;

    MAT3X2F m_renderTM; // ������ ��ȯ ���

    D2D1_RECT_F m_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f);

    std::wstring m_name = L"";

    bool m_isSelected = false;
    bool m_isLeader = false; // ���� �ڽ� ����

    bool m_isSelfRotation = false; // �ڱ� ȸ�� ����

    ComPtr<ID2D1Bitmap1> m_BitmapPtr;

    static int s_id; // static ��� ������ ID�� �����մϴ�. (������)

};

//int BoxObject::s_id = 0;

TransformPracticeScene::~TransformPracticeScene()
{
    for (auto& box : m_BoxObjects)
    {
        delete box;
    }

}

void TransformPracticeScene::SetUp(HWND hWnd)
{
    constexpr int defaultGameObjectCount = 100;

    m_BoxObjects.reserve(defaultGameObjectCount);

    m_hWnd = hWnd;

    SetWindowText(m_hWnd, 
    L"������ �¾�踦 ����� �ּ���. ���� ��Ģ�� ���� �մϴ�. ^^;;");

    std::cout << "�¾��� ������ �ؾ� �մϴ�." << std::endl;
    std::cout << "�༺���� ������ �ϸ� ���ÿ� �¿��� ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�."<< std::endl;
    std::cout << "���� ������ �ϸ鼭 ���ÿ� ������ ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�." << std::endl;
    std::cout << "ȸ�� �ӵ��� �����Ӱ� �����ϼ���." << std::endl;

    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/sun.png", *m_BitmapPtr[0].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/earth.png", *m_BitmapPtr[1].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/moon.png", *m_BitmapPtr[2].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mercury.png", *m_BitmapPtr[3].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/venus.png", *m_BitmapPtr[4].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mars.png", *m_BitmapPtr[5].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/jupiter.png", *m_BitmapPtr[6].GetAddressOf());
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/neptune.png", *m_BitmapPtr[7].GetAddressOf());

    RECT rc;
    if (::GetClientRect(hWnd, &rc))
    {
        float w = static_cast<float>(rc.right - rc.left);
        float h = static_cast<float>(rc.bottom - rc.top);

        m_UnityCamera.SetScreenSize(w, h);
    }

    //�༺ �߰�
    AddBoxObjects(D2D1_POINT_2F{ 450.f , 450.f }, 0);
    m_BoxObjects[0]->SetLeader(true);
    m_SelectedBoxObjects.push_back(m_BoxObjects[0]);

    AddBoxObjects(D2D1_POINT_2F{ 324.f , 666.f }, 1);
    m_BoxObjects[1]->SetParent(m_BoxObjects[0]);

    AddBoxObjects(D2D1_POINT_2F{ 424.f , 666.f }, 2);
    m_BoxObjects[2]->SetParent(m_BoxObjects[1]);
    m_BoxObjects[2]->GetTransform()->SetScale(MYHelper::Vector2F(0.5f, 0.5f));

    AddBoxObjects(D2D1_POINT_2F{ 600.f , 450.f }, 3);
    m_BoxObjects[3]->SetParent(m_BoxObjects[0]);
    m_BoxObjects[3]->GetTransform()->SetScale(MYHelper::Vector2F(0.5f, 0.5f));

    AddBoxObjects(D2D1_POINT_2F{ 550.f , 623.f }, 4);
    m_BoxObjects[4]->SetParent(m_BoxObjects[0]);

    AddBoxObjects(D2D1_POINT_2F{ 150.f , 450.f }, 5);
    m_BoxObjects[5]->SetParent(m_BoxObjects[0]);

    AddBoxObjects(D2D1_POINT_2F{ 275.f , 146.f }, 6);
    m_BoxObjects[6]->SetParent(m_BoxObjects[0]);
    m_BoxObjects[6]->GetTransform()->SetScale(MYHelper::Vector2F(2.f, 2.f));

    AddBoxObjects(D2D1_POINT_2F{ 650.f , 100.f }, 7);
    m_BoxObjects[7]->SetParent(m_BoxObjects[0]);
    m_BoxObjects[7]->GetTransform()->SetScale(MYHelper::Vector2F(1.5f, 1.5f));

}

void TransformPracticeScene::Tick(float deltaTime)
{
   //�Է� �̺�Ʈó��
    ProcessMouseEvents();
    ProcessKeyboardEvents();

    for (auto& box : m_BoxObjects)
    {
        box->Update(deltaTime);
    }

    //ī�޶�

    MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();

    MAT3X2F renderTM = MYTM::MakeRenderMatrix(true);

    MAT3X2F finalTM = renderTM * cameraTM;


    //������

    static myspace::D2DRenderer& globalRenderer = SolarSystemRenderer::Instance();

    wchar_t buffer[128] = L"";
    MYTM::MakeMatrixToString(cameraTM, buffer, 128);

    globalRenderer.RenderBegin();           //beginDraw, ������� �ʱ�ȭ
    globalRenderer.SetTransform(finalTM);   //d2dcontext�� finaltm����

    globalRenderer.DrawRectangle(-10.f, 10.f, 10.f, -10.f, D2D1::ColorF::Red);
    globalRenderer.DrawCircle(0.f, 0.f, 5.f, D2D1::ColorF::Red);
    globalRenderer.DrawMessage(buffer, 10.f, 10.f, 100.f, 100.f, D2D1::ColorF::Black);

    for (auto& box : m_BoxObjects)
    {
        box->Draw(globalRenderer, cameraTM);
    }

    globalRenderer.RenderEnd();
}

void TransformPracticeScene::OnResize(int width, int height)
{ 
    // ������ ũ�� ���� �� ī�޶��� ȭ�� ũ�⸦ ������Ʈ
    m_UnityCamera.SetScreenSize(width, height);

}

void TransformPracticeScene::ProcessKeyboardEvents()
{
    // Ŭ����
    //if (InputManager::Instance().GetKeyPressed(VK_F1))
    //{
    //    ClearBoxObjects();
    //}

    //if (InputManager::Instance().GetKeyPressed(VK_F2))
    //{
    //    SetSelfRotation();
    //}


    // ī�޶� �̵� ó��, 
    static const std::vector<std::pair<int, Vec2>> kCameraMoves = {
      { VK_RIGHT, {  1.f,  0.f } },
      { VK_LEFT,  { -1.f,  0.f } },
      { VK_UP,    {  0.f,  1.f } },
      { VK_DOWN,  {  0.f, -1.f } },
    };

    // C++17���ʹ� structured binding�� ����Ͽ� �� �����ϰ� ǥ���� �� �ֽ��ϴ�.
    for (auto& [vk, dir] : kCameraMoves)
    {
        if (InputManager::Instance().GetKeyDown(vk))
        {
            m_UnityCamera.Move(dir.x, dir.y);
        }
    }

    // ù��° ���õ� �ڽ��� �̵�
    static const std::vector<std::pair<int, Vec2>> kBoxMoves = {
      { 'D', {  1.f,  0.f } }, // DŰ�� ������ �̵�
      { 'A', { -1.f,  0.f } }, // AŰ�� ���� �̵�
      { 'W', {  0.f,  1.f } }, // WŰ�� ���� �̵�
      { 'S', {  0.f, -1.f } }, // SŰ�� �Ʒ��� �̵�
    };

    for (auto& [vk, dir] : kBoxMoves)
    {
        if (InputManager::Instance().GetKeyDown(vk))
        {
            if (m_SelectedBoxObjects.size() > 0)
                m_SelectedBoxObjects.front()->Move(dir);
        }
    }

    // ù��° ���õ� �ڽ��� ȸ��
    if (InputManager::Instance().GetKeyDown(VK_SPACE) && !m_SelectedBoxObjects.empty())
    {
        m_SelectedBoxObjects.front()->Rotate(1.f); // ���� ������ ȸ��
    }

}

void TransformPracticeScene::OnMouseLButtonDown(D2D1_POINT_2F point)
{
    //AddBoxObjects(point, 0);
}

void TransformPracticeScene::OnMouseRButtonDown(D2D1_POINT_2F point)
{
    //SelectBoxObject(point);
}

void TransformPracticeScene::AddBoxObjects(D2D1_POINT_2F point, int i_image)
{
    MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();
    cameraTM.Invert();

    D2D1_POINT_2F worldPt = cameraTM.TransformPoint(point);

    BoxObject* pNewBox = new BoxObject(m_BitmapPtr[i_image]);        //�̹��� ����ְ�

    pNewBox->SetPosition(Vec2(worldPt.x, worldPt.y));       //ī�޶� ������� ��ġ�� �ڽ� ��ġ�� �ְ�

    pNewBox->ToggleSelfRotation();

    m_BoxObjects.push_back(pNewBox);                        //������Ʈ �����̳ʿ� ����ֱ�

    m_objCnt++;
}

void TransformPracticeScene::ClearBoxObjects()
{
    for (auto& box : m_BoxObjects)
    {
        delete box;
    }

    m_BoxObjects.clear();

    m_SelectedBoxObjects.clear();
}

void TransformPracticeScene::SelectBoxObject(D2D1_POINT_2F point)
{
    MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();

    std::list<BoxObject*> oldSelectedList = std::move(m_SelectedBoxObjects);

    std::cout << "size of oldSelectedList: " << oldSelectedList.size() << std::endl;

    for (auto& box : m_BoxObjects)
    {
        if (box->IsHitTest(point, cameraTM))
        {
            box->ToggleSelected();

            if (box->IsSelected()) m_SelectedBoxObjects.push_back(box);
        }
    }

    for (auto it = oldSelectedList.crbegin(); it != oldSelectedList.crend(); ++it)
    {
        (*it)->DetachFromParent();

        if ((*it)->IsSelected())
        {
            m_SelectedBoxObjects.push_front(*it);
        }
        else
        {
            (*it)->SetLeader(false);
        }
    }
    std::cout << "size of m_SelectedBoxObjects: " << m_SelectedBoxObjects.size() << std::endl;

    UpdateRelationship();

}

void TransformPracticeScene::SetSelfRotation()
{
    for (auto& box : m_BoxObjects)
    {
        if (false == box->IsSelected())
        {
            box->ToggleSelfRotation();
        }
    }

}

void TransformPracticeScene::UpdateRelationship()
{
    auto it = m_SelectedBoxObjects.begin();
    if (it == m_SelectedBoxObjects.end()) return; //�ڽ� ������ ����

    (*it)->SetLeader(true);

    if (m_SelectedBoxObjects.size() < 2) return; //�ι�° �ڽ��� ������ ����

    
    while (it != m_SelectedBoxObjects.end() && std::next(it) != m_SelectedBoxObjects.end())
        //���� ���ͷ����Ͱ� ����Ʈ�� ���� �ƴϰ� �� ������ ���� �ƴϸ�
    {
        BoxObject* parent = *it;
        BoxObject* child = *(std::next(it));

        child->SetParent(parent);

        it++;

        std::cout << "�θ�: " << parent->GetTransform()->GetPosition().x << ", "
            << parent->GetTransform()->GetPosition().y
            << " �ڽ�: " << child->GetTransform()->GetPosition().x << ", "
            << child->GetTransform()->GetPosition().y << std::endl;
    }

}

