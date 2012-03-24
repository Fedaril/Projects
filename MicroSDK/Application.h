

#pragma once 

namespace MicroSDK
{

enum ApplicationType
{
	ApplicationType_Console,
	ApplicationType_Window

};

class Application
{
private:
	ApplicationType			m_eApplicationType;

public:
							Application(ApplicationType a_eApplicationType);
	virtual					~Application();

	ApplicationType			GetApplicationType();

	void					Run();
	

	virtual	void			OnInitialize();
	virtual	void			OnShutdown();
	virtual	void			OnUpdate();
	virtual	void			OnRender();
};



inline ApplicationType Application::GetApplicationType()
{
	return m_eApplicationType;
}

} // namespace MicroSDK