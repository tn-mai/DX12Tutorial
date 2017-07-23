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

	virtual bool Load(::Scene::Context&) override;
	virtual bool Unload(::Scene::Context&) override;
	virtual int Update(::Scene::Context&, double delta) override;
	virtual void Draw(Graphics::Graphics& graphics) const override;

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
