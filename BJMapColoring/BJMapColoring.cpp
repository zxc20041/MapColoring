#include "BJMapColoring.h"
#include"settings.h"
#include "audio_thread.h"
using namespace debugger;
using namespace THREAD_SIGNAL;


BJMAP_LOADING_PAGE::BJMAP_LOADING_PAGE()
{
	timeStamp = 0, arc_speed1 = 0, arc_speed2 = 0, arc_angel1 = 0, arc_angel2 = 0;
}

void BJMAP_LOADING_PAGE::update(float timeScale)
{
	timeStamp += frmtm * timeScale;
	arc_speed1 = fabsf(sinf(timeStamp)) * MAX_SPEED + BASE_SPEED;
	arc_speed2 = fabsf(cosf(timeStamp)) * MAX_SPEED + BASE_SPEED;
	arc_angel1 += arc_speed1 * frmtm * timeScale;
	arc_angel2 += arc_speed2 * frmtm * timeScale;
	return;
}

void BJMAP_LOADING_PAGE::rend()
{
	if (arc_angel1 < arc_angel2)
	{
		DrawArc_1(CENTER_POSX, CENTER_POSY, RADIUS, arc_angel1, arc_angel2, 0, g_pBrushBlack, 5);
	}
	else
	{
		DrawArc_1(CENTER_POSX, CENTER_POSY, RADIUS, arc_angel2, arc_angel1, 0, g_pBrushBlack, 5);
	}
	return;
}

BJMAP_PAGE::BJMAP_PAGE():PAGE(PAGE_INDEX, PAGE_CREATED_STATUS, introEff)
{
	hThreadColoring = 0;
	threadColoringSignal = SIGNAL_IDLE;
	timePerStep = new atomic<float>(0);
	coloringNum = new atomic<int>(0);
	returnButton = nullptr;
	startButton = nullptr;
	selectButton = nullptr;
	graph = nullptr;

	targetIndex = 0, colorLimitNum = 4, coloringResult = 0;
}

BJMAP_PAGE::~BJMAP_PAGE()
{
	if (returnButton != nullptr)
	{
		delete returnButton;
	}
	if (startButton != nullptr)
	{
		delete startButton;
	}
	if (selectButton != nullptr)
	{
		delete selectButton;
	}
	delete timePerStep;
	delete coloringNum;
	delete graph;
	
}

bool BJMAP_PAGE::Init()
{
	debugger_main.writelog(DINFO, "in BJMAP_PAGE::Init()", __LINE__);
	if (!PAGE::Init())
	{
		return 0;
	}
	if (returnButton != nullptr)
	{
		debugger_main.writelog(DWARNNING, "returnButton ALREADY created", __LINE__);
	}
	returnButton = new Button(100, 50, 250, 100, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);
	startButton= new Button(50, 250, 200, 300, "Coloring", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
	selectButton = new Button(50, 450, 200, 500, "Select", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
	return 1;
}

void BJMAP_PAGE::Update()
{
	PAGE::Update();
	if (Page_status < PAGE_PREPARED_STATUS)
	{
		if (!g_rm.GetFinishLoading())
		{
			loadingPage.update(1);
			if (returnButton->getClicked())
			{
				g_cm.CreateEffect(9);
				g_PageManager.SwitchPageTo(PAGE_HOME);
				set_target_page(PAGE_HOME, 0, 0);
			}
			return;
		}
		OnLoad();
		selectButton->setClicked();
		Page_status = PAGE_PREPARED_STATUS;
		
	}
	
	if (returnButton->getClicked())
	{
		g_cm.CreateEffect(9);
		g_PageManager.SwitchPageTo(PAGE_HOME);
		set_target_page(PAGE_HOME, 0, 0);
	}

	if (startButton->getClicked())
	{
		graph->resetVertex();
		colorLimitNum = set1[0].color_limit;
		threadColoringSignal.store(SIGNAL_START);
		
	}
	if (selectButton->getClicked())
	{
		if (threadColoringSignal.load() != SIGNAL_RUNNING)
		{
			graph->resetVertex();
			targetIndex++;
			if (targetIndex >= 16)
			{
				targetIndex = 0;
			}
			graph->setVisited(targetIndex);
		}
		
	}
	return;
}

void BJMAP_PAGE::FixedUpdate()
{
}

void BJMAP_PAGE::Rend()
{
	g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));
	FillRectangle_1(0, 0, 1600, 900, g_pBrushLightBlue, 0.2f);
	if (Page_status < PAGE_PREPARED_STATUS)
	{
		loadingPage.rend();
		DrawTextA_1(lan[0].loading, g_rm.getFont("DEYIHEI"), 600, 600, 1000, 750, g_pBrushBlack);
		return;
	}

	auto ori_mode=g_pD2DDeviceContext->GetAntialiasMode();
	g_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	for (int i = 0; i < 16; i++)
	{
		if (graph->getVisited(i))
		{
			FillOpacityMask_1(0, 0, 1600, 900, BrushRand[nodes[i].color_index], g_rm.getTexture(nodes[i].aliasName), 0.5f);
		}
		else
		{
			DrawBitmap_1(g_rm.getTexture(nodes[i].aliasName), D2D1::RectF(0, 0, 1600, 900), 0.2f);
		}
	}
	DrawTextA_1(to_string(coloringNum->load()), g_rm.getFont("Quarlow"), 700, 20, 900, 70, g_pBrushBlack);

	g_pD2DDeviceContext->SetAntialiasMode(ori_mode);
	return;
	
}

bool BJMAP_PAGE::EnterPage()
{
	debugger_main.writelog(DINFO, "IN BJMAP_PAGE::EnterPage()", __LINE__);
	if (Page_status == PAGE_CREATED_STATUS)
	{
		if (!Init())
		{
			debugger_main.writelog(DERROR, "PAGE::Init() failed in BJMAP_PAGE", __LINE__);
		}
		Page_status = PAGE_INIT_STATUS;
	}
	if (Page_status != PAGE_INIT_STATUS)
	{
		if (!ExitPage())
		{
			debugger_main.writelog(DWARNNING, "PAGE::ExitPage() failed in BJMAP_PAGE", __LINE__);
		}
	}
	if (!PAGE::EnterPage())
	{
		debugger_main.writelog(DERROR, "PAGE::EnterPage() failed in BJMAP_PAGE", __LINE__);
	}
	g_rm.AddResource("changping", ".\\pic\\map_mask\\changping.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("chaoyang", ".\\pic\\map_mask\\chaoyang.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("daxing", ".\\pic\\map_mask\\daxing.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("dongcheng", ".\\pic\\map_mask\\dongcheng.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("fangshan", ".\\pic\\map_mask\\fangshan.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("fengtai", ".\\pic\\map_mask\\fengtai.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("haidian", ".\\pic\\map_mask\\haidian.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("huairou", ".\\pic\\map_mask\\huairou.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("mentougou", ".\\pic\\map_mask\\mentougou.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("miyun", ".\\pic\\map_mask\\miyun.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("pinggu", ".\\pic\\map_mask\\pinggu.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("shijingshan", ".\\pic\\map_mask\\shijingshan.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("shunyi", ".\\pic\\map_mask\\shunyi.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("tongzhou", ".\\pic\\map_mask\\tongzhou.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("xicheng", ".\\pic\\map_mask\\xicheng.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);
	g_rm.AddResource("yanqing", ".\\pic\\map_mask\\yanqing.png", ResourceManager::ResourceType::Resource_Texture, "pass", RESOURCE_INFO::DEFAULT_Bitmap_WicBitmap);

	g_rm.AddResource("Mapinfo", ".\\data\\graph.dat", ResourceManager::ResourceType::Resource_Text, "pass", TEXTURE_DESC(), FONT_DESC(), {32,1});

	g_rm.LoadAll();
	
	g_cm.AddButton(returnButton);
	g_cm.AddButton(startButton); 
	g_cm.AddButton(selectButton);

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			nodes[i].strbuf[j].clear();
		}
	}
	threadColoringSignal.store(SIGNAL_IDLE);
	timePerStep->store(set1[0].demo_mode_timePerStep*0.05f);
	coloringNum->store(0);
	CLASS_TYPE* type = new CLASS_TYPE();
	type->type = CLASS_TYPE::CLASS_BJMAP_PAGE;
	type->Object = this;
	hThreadColoring= (HANDLE)_beginthreadex(NULL, 0, Thread_Coloring, type, 0, NULL);

	if (hThreadColoring == 0)
	{
		debugger_main.writelog(DWARNNING, "_beginthreadex Thread_Coloring failed", __LINE__);
	}
	
	debugger_main.writelog(DINFO, "FINISHING BJMAP_PAGE::EnterPage()", __LINE__);
	
	return 1;
}

bool BJMAP_PAGE::ExitPage()
{
	debugger_main.writelog(DINFO, "IN BJMAP_PAGE::ExitPage()", __LINE__);
	threadColoringSignal.store(SIGNAL_QUIT);
	coloringNum->store(-100);
	timePerStep->store(0);
	g_rm.ReleaseAll();
	
	if (hThreadColoring != 0)
	{
		DWORD dz = WaitForSingleObject(hThreadColoring, 200);
		switch (dz)
		{
		case WAIT_OBJECT_0:
		case WAIT_FAILED:
			break;
		case WAIT_TIMEOUT:
			debugger_main.writelog(DWARNNING, "Terminate hThreadColoring while timeout", __LINE__);
			TerminateThread(hThreadColoring, 100);    //线程无响应
			break;
		default:
			break;
		}
	}
	if (graph != nullptr)
	{
		delete graph;
		graph = nullptr;
	}
	Page_status = PAGE_INIT_STATUS;
	return 1;
}

void BJMAP_PAGE::OnLoad()
{
	TEXT_RESOURCE* text = g_rm.getText("Mapinfo");
	int cnt = 0, index = 0;
	if (text == nullptr || text->lines < 16)
	{
		debugger_main.writelog(DERROR, "text lines not match in Mapinfo", __LINE__);
		return;
	}
	for (int i = 0; i < 16; i++)
	{
		while (text->content[i][cnt] != '\0')
		{
			if (text->content[i][cnt] == ' ')
			{
				index++;
				cnt++;
				continue;
			}
			nodes[i].strbuf[index] += text->content[i][cnt];
			cnt++;
		}
		cnt = 0;
		index = 0;
	}
	graph = new MyGraph<MAP_NODE*>(32);
	//解析数据
	for (int i = 0; i < 16; i++)
	{
		nodes[i].aliasName = nodes[i].strbuf[1];
		nodes[i].color_index = rand() % 8;
		graph->addVertex(&nodes[i]);
		
	}
	int edge_total_num = 0;
	for (int i = 0; i < 16; i++)
	{
		int edge_num = atoi(nodes[i].strbuf[2].c_str());
		for (int j = 0; j < edge_num; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				if (nodes[i].strbuf[3 + j] == nodes[k].aliasName)
				{
					graph->setValue2Matrix4UndirectedGraph(i, k);
					edge_total_num++;
					break;
				}
				if (k == 15)
				{
					debugger_main.writelog(DWARNNING, "edge name not found " + nodes[i].strbuf[3 + j]);
				}
			}
		}
		nodes[i].name = nodes[i].strbuf[3 + edge_num];
	}
	targetIndex = rand() % 16;
	debugger_main.writelog(DINFO, "added edge_total_num= " + to_string(edge_total_num));
	return;
}

unsigned __stdcall Thread_Coloring(LPVOID lpParameter)
{
	CLASS_TYPE* data = (CLASS_TYPE*)lpParameter;
	if (data->type == CLASS_TYPE::CLASS_BJMAP_PAGE)
	{
		BJMAP_PAGE* page = (BJMAP_PAGE*)data->Object;
		delete data;
		Sleep(50);
		page->threadColoringSignal.store(SIGNAL_PREPARED);
		while (1)
		{
			if (page->threadColoringSignal.load() == SIGNAL_QUIT || page->threadColoringSignal.load() == SIGNAL_IDLE || normal_quit || quit_single)
			{
				page->threadColoringSignal.store(SIGNAL_STOPPED);
				return 0;
			}
			if (page->threadColoringSignal.load() == SIGNAL_START)
			{
				page->threadColoringSignal.store(SIGNAL_RUNNING);
				page->coloringNum->store(0);
				debugger_main.writelog(DINFO, "Thread_Coloring start coloring", __LINE__);
				if (page->graph->DFS(page->targetIndex, page->colorLimitNum, page->timePerStep, page->coloringNum))
				{
					page->coloringResult = 1;
					g_am.playEffectSound(9);
					debugger_main.writelog(DINFO, "Thread_Coloring coloring succeed at " + to_string(page->targetIndex), __LINE__);
				}
				else
				{
					page->coloringResult = 0;
					g_am.playEffectSound(6);
					debugger_main.writelog(DINFO, "Thread_Coloring coloring failed at " + to_string(page->targetIndex), __LINE__);
				} 
				if (page->threadColoringSignal.load() == SIGNAL_QUIT)
				{
					page->threadColoringSignal.store(SIGNAL_STOPPED);
					return 0;
				}
				page->threadColoringSignal.store(SIGNAL_PREPARED);
			}
			Sleep(10);
		}
	}
	else if (data->type == CLASS_TYPE::CLASS_MAP_COLORING_PAGE)
	{
		MAP_COLORING_PAGE* page = (MAP_COLORING_PAGE*)data->Object;
		delete data;
		Sleep(50);
		page->threadColoringSignal.store(SIGNAL_PREPARED);
		while (1)
		{
			if (page->threadColoringSignal.load() == SIGNAL_QUIT || page->threadColoringSignal.load() == SIGNAL_IDLE || normal_quit || quit_single)
			{
				page->threadColoringSignal.store(SIGNAL_STOPPED);
				return 0;
			}
			if (page->threadColoringSignal.load() == SIGNAL_START)
			{
				page->coloringNum->store(0);
				page->threadColoringSignal.store(SIGNAL_RUNNING);
				page->coloringNum->store(0);
				debugger_main.writelog(DINFO, "Thread_Coloring start coloring", __LINE__);
				page->timer.start_timer();
				if (page->graph->DFS(page->targetIndex, page->colorLimitNum, page->timePerStep, page->coloringNum,page->depth))
				{
					page->coloringResult = 1;
					g_am.playEffectSound(9);
					debugger_main.writelog(DINFO, "Thread_Coloring coloring succeed at " + to_string(page->targetIndex) + " while colorLimitNum= " + to_string(page->colorLimitNum), __LINE__);
				}
				else
				{
					page->coloringResult = 0;
					g_am.playEffectSound(6);
					debugger_main.writelog(DINFO, "Thread_Coloring coloring failed at " + to_string(page->targetIndex)+" while colorLimitNum= "+to_string(page->colorLimitNum), __LINE__);
				}
				if (page->threadColoringSignal.load() == SIGNAL_QUIT)
				{
					page->threadColoringSignal.store(SIGNAL_STOPPED);
					return 0;
				}
				if (page->threadColoringSignal.load() == SIGNAL_START)
				{
					page->graph->resetVertex();
					continue;
				}
				page->depth->store(0);
				page->threadColoringSignal.store(SIGNAL_PREPARED);
			}
			Sleep(10);
		}
	}
	
	
}



MAP_COLORING_PAGE::MAP_COLORING_PAGE() :PAGE(PAGE_INDEX, PAGE_CREATED_STATUS, introEff)
{
	hThreadColoring = 0;
	threadColoringSignal = SIGNAL_IDLE;
	timePerStep = new atomic<float>(0);
	coloringNum = new atomic<int>(0);
	depth = new atomic<int>(0);
	returnButton = nullptr;
	startButton = nullptr;
	timeButton = nullptr;
	graph = nullptr;

	nodes = nullptr;
	targetIndex = 0, colorLimitNum = 4, coloringResult = 0;
	blocks = nullptr;
	nodes_num = 0, block_num = 0;
	lines_num = 0, cols_num = 0;
}

MAP_COLORING_PAGE::~MAP_COLORING_PAGE()
{
	if (returnButton != nullptr)
	{
		delete returnButton;
	}
	if (startButton != nullptr)
	{
		delete startButton;
	}
	if (timeButton != nullptr)
	{
		delete timeButton;
	}
	delete timePerStep;
	delete coloringNum;
	delete graph;

	if (nodes != nullptr)
	{
		delete[] nodes;
	}
	if (blocks != nullptr)
	{
		delete[] blocks;
	}
}

bool MAP_COLORING_PAGE::Init()
{
	if (!PAGE::Init())
	{
		return 0;
	}
	returnButton = new Button(20, 50, 150, 100, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);
	startButton = new Button(20, 250, 150, 300, "Coloring", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
	timeButton = new Button(20, 350, 150, 400, "Speed", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
	return 1;
}

void MAP_COLORING_PAGE::Update()
{
	PAGE::Update();
	if (Page_status < PAGE_PREPARED_STATUS)
	{
		if (!g_rm.GetFinishLoading())
		{
			//debugger_main.writelog(DINFO, "in MAP_COLORING_PAGE::Update() waiting FinishLoading", __LINE__);
			loadingPage.update(1);
			if (returnButton->getClicked())
			{
				g_cm.CreateEffect(9);
				g_PageManager.SwitchPageTo(PAGE_HOME);
				set_target_page(PAGE_HOME, 0, 0);
			}
			return;
		}
		
		OnLoad();
		Page_status = PAGE_PREPARED_STATUS;

	}

	if (returnButton->getClicked())
	{
		g_cm.CreateEffect(9);
		g_PageManager.SwitchPageTo(PAGE_HOME);
		set_target_page(PAGE_HOME, 0, 0);
	}

	if (startButton->getClicked())
	{
		graph->resetVertex();
		targetIndex = rand() % nodes_num;
		colorLimitNum = set1[0].color_limit;
		coloringNum->store(-100);
		depth->store(0);
		threadColoringSignal.store(SIGNAL_START);
		
	}
	if (timeButton->getClicked())
	{
		if (timePerStep->load() == 0)
		{
			timePerStep->store(set1[0].extra_mode_timePerStep * 0.001f);
		}
		else
		{
			timePerStep->store(0);
		}
	}
	return;
}

void MAP_COLORING_PAGE::FixedUpdate()
{
	return;
}

void MAP_COLORING_PAGE::Rend()
{
	g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));
	FillRectangle_1(0, 0, 1600, 900, g_pBrushLightBlue, 0.2f);
	if (Page_status < PAGE_PREPARED_STATUS)
	{
		loadingPage.rend();
		DrawTextA_1(lan[0].loading, g_rm.getFont("DEYIHEI"), 600, 600, 1000, 750, g_pBrushBlack);
		return;
	}
	for (int i = 0; i < nodes_num; i++)
	{
		nodes[i].rend(graph->getVisited(i));
	}
	static float time_used = 0;
	if (threadColoringSignal.load() == SIGNAL_RUNNING)
	{
		time_used = timer.stop_timer();
	}
	DrawTextA_1("count: "+to_string(coloringNum->load()), g_rm.getFont("Quarlow"), 500, 10, 1100, 50, g_pBrushBlack);
	DrawTextA_1("depth: " + to_string(depth->load())+" / "+to_string(nodes_num), g_rm.getFont("Quarlow"), 800, 850, 1400, 890, g_pBrushBlack);
	DrawTextA_1("time: " + to_string((int)time_used)+ to_string(time_used-(int)time_used).substr(1,3)+"s", g_rm.getFont("Quarlow"), 200, 850, 800, 890, g_pBrushBlack);
	return;
}

bool MAP_COLORING_PAGE::EnterPage()
{
	debugger_main.writelog(DINFO, "IN MAP_COLORING_PAGE::EnterPage()", __LINE__);
	if (Page_status == PAGE_CREATED_STATUS)
	{
		if (!Init())
		{
			debugger_main.writelog(DERROR, "PAGE::Init() failed in MAP_COLORING_PAGE", __LINE__);
		}
		Page_status = PAGE_INIT_STATUS;
	}
	if (Page_status != PAGE_INIT_STATUS)
	{
		if (!ExitPage())
		{
			debugger_main.writelog(DWARNNING, "PAGE::ExitPage() failed in MAP_COLORING_PAGE", __LINE__);
		}
	}
	if (!PAGE::EnterPage())
	{
		debugger_main.writelog(DERROR, "PAGE::EnterPage() failed in MAP_COLORING_PAGE", __LINE__);
	}
	g_rm.LoadAll();

	g_cm.AddButton(returnButton); 
	g_cm.AddButton(startButton);
	g_cm.AddButton(timeButton);

	threadColoringSignal.store(SIGNAL_IDLE);
	timePerStep->store(set1[0].extra_mode_timePerStep*0.001f);
	coloringNum->store(0);
	CLASS_TYPE* type = new CLASS_TYPE();
	type->type = CLASS_TYPE::CLASS_MAP_COLORING_PAGE;
	type->Object = this;
	hThreadColoring = (HANDLE)_beginthreadex(NULL, 0, Thread_Coloring, type, 0, NULL);
	if (hThreadColoring == 0)
	{
		debugger_main.writelog(DWARNNING, "_beginthreadex Thread_Coloring failed", __LINE__);
	}
	else
	{
		SetThreadPriority(hThreadColoring, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	
	if (set1[0].extra_mode_size == 2)
	{
		cols_num = 140, lines_num = 80;
		nodes_num = cols_num * lines_num / 4, block_num = cols_num * lines_num;
	}
	else if (set1[0].extra_mode_size == 1)
	{
		cols_num = 70, lines_num = 40;
		nodes_num = cols_num * lines_num / 4, block_num = cols_num * lines_num;
	}
	else
	{
		cols_num = 42, lines_num = 24;
		nodes_num = cols_num * lines_num / 8, block_num = cols_num * lines_num;
	}

	debugger_main.writelog(DINFO, "FINISHING MAP_COLORING_PAGE::EnterPage()", __LINE__);

	return 1;
}

bool MAP_COLORING_PAGE::ExitPage()
{
	debugger_main.writelog(DINFO, "IN MAP_COLORING_PAGE::ExitPage()", __LINE__);
	threadColoringSignal.store(SIGNAL_QUIT);
	coloringNum->store(-100);
	
	g_rm.ReleaseAll();
	
	
	if (hThreadColoring != 0)
	{
		DWORD dz = WaitForSingleObject(hThreadColoring, 200);
		switch (dz) 
		{
		case WAIT_OBJECT_0:
		case WAIT_FAILED:
			break;
		case WAIT_TIMEOUT:
			debugger_main.writelog(DWARNNING, "Terminate hThreadColoring while timeout", __LINE__);
			TerminateThread(hThreadColoring, 100);    //线程无响应
			break;
		default:
			break;
		}
	}

	if (graph != nullptr)
	{
		delete graph;
		graph = nullptr;
	}
	if (nodes != nullptr)
	{
		delete[]nodes;
		nodes = nullptr;
	}
	if (blocks != nullptr)
	{
		delete[] blocks;
		blocks = nullptr;
	}
	Page_status = PAGE_INIT_STATUS;
	return 1;
}

void MAP_COLORING_PAGE::OnLoad()
{
	//debugger_main.writelog(DINFO, "in MAP_COLORING_PAGE::OnLoad()", __LINE__);
	graph = new MyGraph<MAP_NODE*>(nodes_num);
	nodes = new MAP_NODE[nodes_num];
	blocks = new MAP_BLOCK[block_num];
	for (int i = 0; i < block_num; i++)
	{
		blocks[i].fa = i;
	}
	int current_node_num = block_num;
	
	while (current_node_num > nodes_num)
	{
		//merge blocks
		if (mergeBlock(rand() % block_num))
		{
			current_node_num--;
		}
	}
	debugger_main.writelog(DINFO, "in MAP_COLORING_PAGE::OnLoad() block_num= " + to_string(block_num) + " nodes_num= " + to_string(nodes_num), __LINE__);
	
	int current_node_index = 0;
	for (int i = 0; i < block_num; i++)
	{
		blocks[i].posx1 = 150 + i % cols_num * 1400.f / cols_num;
		blocks[i].posx2 = 150 + (i % cols_num + 1) * 1400.f / cols_num;
		blocks[i].posy1 = (i / cols_num) * 800.f / lines_num + 50;
		blocks[i].posy2 = (i / cols_num + 1) * 800.f / lines_num + 50;
		if (blocks[i].fa == i)
		{
			if (current_node_index >= nodes_num)
			{
				debugger_main.writelog(DWARNNING, "current_node_index reach nodes_num while block at= " + to_string(i) + " compared to target= " + to_string(block_num-1), __LINE__);
				continue;
			}
			int r = dfsAddToNode(i, current_node_index);
			//debugger_main.writelog(DINFO, "block index= "+to_string(i)+" dfsAddToNode node_index= " + to_string(current_node_index) + " blocks_num= " + to_string(r), __LINE__);
			current_node_index++;
		}
	}
	debugger_main.writelog(DINFO, "node num added= " + to_string(current_node_index)+" compared to target= "+to_string(nodes_num), __LINE__);

	//add vertexs
	for (int i = 0; i < nodes_num; i++)
	{
		graph->addVertex(&nodes[i]);
	}

	for (int i = 0; i < block_num; i++)
	{
		connectEdge(i);
	}
	debugger_main.writelog(DINFO, "finishing MAP_COLORING_PAGE::OnLoad()", __LINE__);
	return;
}

int MAP_COLORING_PAGE::getFa(int index)
{
	return blocks[index].fa == index ? index : getFa(blocks[index].fa);
}

bool MAP_COLORING_PAGE::mergeBlock(int index)
{
	int startIndex = rand() % 4;
	const int neighber_block_index[4] = { index - cols_num  ,index - 1,index + cols_num,index + 1 };
	for (int i = 0; i < 4; i++)
	{
		int currentIndex = (startIndex + i) % 4;
		if (currentIndex == 0)
		{
			if (index - cols_num > 0)
			{
				if (getFa(neighber_block_index[currentIndex]) != getFa(index))
				{
					blocks[getFa(index)].fa = getFa(neighber_block_index[currentIndex]);
					return 1;
				}
			}
		}
		else if (currentIndex == 1)
		{
			if (index % cols_num != 0)
			{
				if (getFa(neighber_block_index[currentIndex]) != getFa(index))
				{
					blocks[getFa(index)].fa = getFa(neighber_block_index[currentIndex]);
					return 1;
				}
			}
		}
		else if (currentIndex == 2)
		{
			if (index + cols_num < block_num)
			{
				if (getFa(neighber_block_index[currentIndex]) != getFa(index))
				{
					blocks[getFa(index)].fa = getFa(neighber_block_index[currentIndex]);
					return 1;
				}
			}
		}
		else
		{
			if ((index + 1) % cols_num != 0)
			{
				if (getFa(neighber_block_index[currentIndex]) != getFa(index))
				{
					blocks[getFa(index)].fa = getFa(neighber_block_index[currentIndex]);
					return 1;
				}
			}
		}
	}
	return 0;
}

void MAP_COLORING_PAGE::connectEdge(int index)
{
	const int neighber_block_index[] = {index + 1,index + cols_num };
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			if ((index + 1) % cols_num == 0)
			{
				continue;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
		}
		else
		{
			if (neighber_block_index[i] >= block_num)
			{
				continue;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
		}
		/*switch (i)
		{
		case 0:
			if ((index + 1) % cols_num == 0)
			{
				break;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
			break;
		case 1:
			if (neighber_block_index[i] >= block_num || index % cols_num == 0)
			{
				break;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
			break;
		case 2:
			if (neighber_block_index[i] >= block_num)
			{
				break;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
			break;
		case 3:
			if (neighber_block_index[i] >= block_num || (index + 1) % cols_num == 0)
			{
				break;
			}
			if (blocks[index].node_index != blocks[neighber_block_index[i]].node_index)
			{
				graph->setValue2Matrix4UndirectedGraph(blocks[index].node_index, blocks[neighber_block_index[i]].node_index);
			}
			break;
		default:
			break;
		}*/
	}
	return;
}

int MAP_COLORING_PAGE::dfsAddToNode(int blockIndex, int nodeIndex)
{
	blocks[blockIndex].isVisited = 1;
	blocks[blockIndex].node_index = nodeIndex;
	nodes[nodeIndex].block_included.push_back(&blocks[blockIndex]);
	int r = 1;
	if (blockIndex - cols_num >= 0)	//up
	{
		if (getFa(blockIndex) == getFa(blockIndex - cols_num))
		{
			blocks[blockIndex].edges[0] = 0;
			blocks[blockIndex - cols_num].edges[1] = 0;
			if (!blocks[blockIndex - cols_num].isVisited)
			{
				r+=dfsAddToNode(blockIndex - cols_num, nodeIndex);
			}
		}
	}
	if (blockIndex + cols_num < block_num)	//down
	{
		if (getFa(blockIndex) == getFa(blockIndex + cols_num))
		{
			blocks[blockIndex].edges[1] = 0;
			blocks[blockIndex + cols_num].edges[0] = 0;
			if (!blocks[blockIndex + cols_num].isVisited)
			{
				r+=dfsAddToNode(blockIndex + cols_num, nodeIndex);
			}
		}
	}
	if (blockIndex % cols_num != 0)	//left
	{
		if (getFa(blockIndex) == getFa(blockIndex - 1))
		{
			blocks[blockIndex].edges[2] = 0;
			blocks[blockIndex - 1].edges[3] = 0;
			if (!blocks[blockIndex - 1].isVisited)
			{
				r+=dfsAddToNode(blockIndex - 1, nodeIndex);
			}
		}
	}
	if ((blockIndex + 1) % cols_num != 0)	//right
	{
		if (getFa(blockIndex) == getFa(blockIndex + 1))
		{
			blocks[blockIndex].edges[3] = 0;
			blocks[blockIndex + 1].edges[2] = 0;
			if (!blocks[blockIndex + 1].isVisited)
			{
				dfsAddToNode(blockIndex + 1, nodeIndex);
			}
		}
	}
	return r;
}

void MAP_COLORING_PAGE::MAP_NODE::rend(bool isVisitedInGraph)
{
	if (isVisitedInGraph)
	{
		for (auto& i : block_included)
		{
			FillRectangle_1(i->posx1, i->posy1, i->posx2, i->posy2, BrushRand[color_index], 0.75f);
			if (i->edges[0])
			{
				DrawLine_1(i->posx1, i->posy1, i->posx2, i->posy1, (i->posy2 - i->posy1) * 0.1f, g_pBrushBlack, 0.25f);
			}
			if (i->edges[1])
			{
				DrawLine_1(i->posx1, i->posy2, i->posx2, i->posy2, (i->posy2 - i->posy1) * 0.1f, g_pBrushBlack, 0.25f);
			}
			if (i->edges[2])
			{
				DrawLine_1(i->posx1, i->posy1, i->posx1, i->posy2, (i->posx2 - i->posx1) * 0.1f, g_pBrushBlack, 0.25f);
			}
			if (i->edges[3])
			{
				DrawLine_1(i->posx2, i->posy1, i->posx2, i->posy2, (i->posx2 - i->posx1) * 0.1f, g_pBrushBlack, 0.25f);
			}
		}
	}
	else
	{
		for (auto& i : block_included)
		{
			FillRectangle_1(i->posx1, i->posy1, i->posx2, i->posy2, g_pBrushGray, 0.25f);
			if (i->edges[0])
			{
				DrawLine_1(i->posx1, i->posy1, i->posx2, i->posy1, (i->posy2 - i->posy1) * 0.05f, g_pBrushBlack, 0.2f);
			}
			if (i->edges[1])
			{
				DrawLine_1(i->posx1, i->posy2, i->posx2, i->posy2, (i->posy2 - i->posy1) * 0.05f, g_pBrushBlack, 0.2f);
			}
			if (i->edges[2])
			{
				DrawLine_1(i->posx1, i->posy1, i->posx1, i->posy2, (i->posx2 - i->posx1) * 0.05f, g_pBrushBlack, 0.2f);
			}
			if (i->edges[3])
			{
				DrawLine_1(i->posx2, i->posy1, i->posx2, i->posy2, (i->posx2 - i->posx1) * 0.05f, g_pBrushBlack, 0.2f);
			}
		}
	}
	return;
}

void MAP_COLORING_PAGE::MAP_BLOCK::rend_dbg()
{
	FillRectangle_1(posx1, posy1, posx2, posy2, g_pBrushGray, 0.25f);
	if (edges[0])
	{
		DrawLine_1(posx1, posy1, posx2, posy1, (posy2 - posy1) * 0.05f, g_pBrushBlack, 0.2f);
	}
	if (edges[1])
	{
		DrawLine_1(posx1, posy2, posx2, posy2, (posy2 - posy1) * 0.05f, g_pBrushBlack, 0.2f);
	}
	if (edges[2])
	{
		DrawLine_1(posx1, posy1, posx1, posy2, (posx2 - posx1) * 0.05f, g_pBrushBlack, 0.2f);
	}
	if (edges[3])
	{
		DrawLine_1(posx2, posy1, posx2, posy2, (posx2 - posx1) * 0.05f, g_pBrushBlack, 0.2f);
	}
	return;
}
