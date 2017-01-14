/**
* @file MainGameScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
#define DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"
#include "../Action.h"

class MainGameScene : public Scene::Scene
{
public:
	enum ExitCode
	{
		ExitCode_Ending,
		ExitCode_Pause,
		ExitCode_GameOver,
	};

	static ::Scene::ScenePtr Create();

	virtual bool Load();
	virtual bool Unload();
	virtual int Update(double delta);
	virtual void Draw(::Scene::Graphics& graphics) const;

private:
	MainGameScene();
	MainGameScene(const MainGameScene&) = delete;
	MainGameScene& operator=(const MainGameScene&) = delete;

	Resource::Texture texBackground;
	Resource::Texture texObjects;
	Resource::Texture texFont;
	std::vector<Sprite::Sprite> sprBackground;
	std::vector<Sprite::Sprite> sprObjects;
	std::vector<Sprite::Sprite> sprFont;
	Sprite::FilePtr cellFile[2];
	AnimationFile animationFile[2];
	Action::FilePtr actionFile;
	double time;
};

#endif // DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
