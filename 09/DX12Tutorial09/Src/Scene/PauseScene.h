/**
* @file PauseScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_PAUSESCENE_H_
#define DX12TUTORIAL_SRC_SCENE_PAUSESCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"

class PauseScene : public Scene::Scene
{
public:
	static ::Scene::ScenePtr Create();

	virtual bool Load();
	virtual bool Unload();
	virtual int Update(double delta);
	virtual void Draw(::Scene::Graphics& graphics) const;

private:
	PauseScene();
	PauseScene(const PauseScene&) = delete;
	PauseScene& operator=(const PauseScene&) = delete;

	Resource::Texture texFont;
	std::vector<Sprite::Sprite> sprFont;
	Sprite::FilePtr cellFile;
	AnimationFile animationFile;
	double time;
};

#endif // DX12TUTORIAL_SRC_SCENE_PAUSESCENE_H_
