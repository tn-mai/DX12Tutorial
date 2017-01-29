/**
* @file GameOverScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_GAMEOVERSCENE_H_
#define DX12TUTORIAL_SRC_SCENE_GAMEOVERSCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"

class GameOverScene : public Scene::Scene
{
public:
	static ::Scene::ScenePtr Create();

	virtual bool Load();
	virtual bool Unload();
	virtual int Update(double delta);
	virtual void Draw(Graphics::Graphics& graphics) const;

private:
	GameOverScene();
	GameOverScene(const GameOverScene&) = delete;
	GameOverScene& operator=(const GameOverScene&) = delete;

	Resource::Texture texBackground;
	Resource::Texture texFont;
	std::vector<Sprite::Sprite> sprBackground;
	std::vector<Sprite::Sprite> sprFont;
	Sprite::FilePtr cellFile;
	AnimationFile animationFile;
	double time;
};

#endif // DX12TUTORIAL_SRC_SCENE_GAMEOVERSCENE_H_
