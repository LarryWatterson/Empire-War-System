/***
@LWT
***/
#include "stdafx.h"
#ifdef ENABLE_IMPARATORLUK_SAVASI
#include "char.h"
#include "lwt_imparatorluksavas.h"
#include "sectree_manager.h"
#include "questmanager.h"
#include "config.h" /* PER_SEC */
#ifdef ENABLE_AUTO_NOTICE_SYSTEM
#include "auto_notice_manager.h"
#endif

static LPEVENT koye_gitme_event = NULL, imp_hazirlik_event = NULL;

namespace // nonpublic islemler
{
	const char* IMP_TEXT_MAP(BYTE gelen) // duyuru yazilari locale_string
	{
		typedef std::map<BYTE, const char *> TMapNotice;
		TMapNotice IMP_TEXT_MAP;
		IMP_TEXT_MAP[0] = LC_TEXT("%dsonrasabaslayacakimp");
		IMP_TEXT_MAP[1] = LC_TEXT("impekbilgiler%d%d%d");
		IMP_TEXT_MAP[2] = LC_TEXT("is_basladi");
		IMP_TEXT_MAP[3] = LC_TEXT("imp_kazanan%s");

		return IMP_TEXT_MAP[gelen];
	}

	EVENTINFO(KoyeGitmeTimerInfo)
	{
		CImparatorlukSavasi* pEvents;

		KoyeGitmeTimerInfo()
			: pEvents(0)
		{
		}
	};

	EVENTFUNC(koye_gitme_timer)
	{
		if (event == NULL)
			return 0;

		if (event->info == NULL)
			return 0;

		KoyeGitmeTimerInfo* info = dynamic_cast<KoyeGitmeTimerInfo*>(event->info);

		if (info == NULL)
			return 0;

		CImparatorlukSavasi* pInstance = info->pEvents;

		if (pInstance == NULL)
			return 0;

		CImparatorlukSavasi::instance().EventBittiYallahKoye();

		event_cancel(&koye_gitme_event);
		koye_gitme_event = NULL;

		return 0;
	}

	template<typename T>
	void TimerBaslat(T t)
	{
		KoyeGitmeTimerInfo* info = AllocEventInfo<KoyeGitmeTimerInfo>();
		info->pEvents = t;

		koye_gitme_event = event_create(koye_gitme_timer, info, PASSES_PER_SEC(10));
	}

	EVENTINFO(ImpSavasiTimerInfo)
	{
		int		left_second;

		ImpSavasiTimerInfo()
			:left_second(0)
		{
		}
	};


	EVENTFUNC(imp_savas_timer)
	{
		if (event == NULL)
			return 0;

		if (event->info == NULL)
			return 0;

		ImpSavasiTimerInfo* info = dynamic_cast<ImpSavasiTimerInfo*>(event->info);

		if (info == NULL)
		{
			sys_err("<ImpSavasiTimerInfo> <Factor> Null pointer");
			return 0;
		}

		if (info->left_second <= 0)
		{
			event_cancel(&imp_hazirlik_event);
			imp_hazirlik_event = NULL;
			CAutoNotice::instance().SendNoticeLine(2, IMP_TEXT_MAP(2));
			quest::CQuestManager::instance().RequestSetEventFlag("imp_savasi", 1);
			return 0;
		}
		else
		{
			CAutoNotice::instance().SendNoticeLine(1, IMP_TEXT_MAP(0), info->left_second);
			CAutoNotice::instance().SendNoticeLine(1, IMP_TEXT_MAP(1), CImparatorlukSavasi::instance().GetMaxLevel(), CImparatorlukSavasi::instance().GetMaxKatilim(), CImparatorlukSavasi::instance().GetMaxSkor());
		}
		--info->left_second;
		return PASSES_PER_SEC(1);
	}

	struct FCikDisariCik
	{
		void operator()(LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;

				if (!ch->IsPC() || ch->GetGMLevel() >= 3)
					return;

				if (ch->IsPC()) {
					ch->ExitToSavedLocation();
					ch->GoHome();
				}
				CImparatorlukSavasi::instance().VerileriTemizle();
			}
		}
	};
}

void CImparatorlukSavasi::TImpData::Initialize()
{
	bNo = 0;
	dwPuan = 0;
	dwKatilimci = 0;
}

void CImparatorlukSavasi::TImpData::KillSay()
{
	++dwPuan;
	if (dwPuan >= CImparatorlukSavasi::instance().GetMaxSkor())
		CImparatorlukSavasi::instance().KazananiDuyur(bNo);
}

void CImparatorlukSavasi::TImpData::OyuncuEkle()
{
	++dwKatilimci;
}

void CImparatorlukSavasi::TImpData::OyuncuSil()
{
	--dwKatilimci;
}

void CImparatorlukSavasi::EventBaslat()
{
	if (imp_hazirlik_event != NULL)
	{
		event_cancel(&imp_hazirlik_event);
		imp_hazirlik_event = NULL;
	}

	ImpSavasiTimerInfo* info = AllocEventInfo<ImpSavasiTimerInfo>();
	info->left_second = 5; // 5dk duyuru
	imp_hazirlik_event = event_create(imp_savas_timer, info, PASSES_PER_SEC(1));

/* sabit */
	m_ImpData[0].bNo = 1;
	m_ImpData[1].bNo = 2;
	m_ImpData[2].bNo = 3;
}

void CImparatorlukSavasi::VerileriTemizle()
{
	for (BYTE i = 0; i < KRALLIK_SAYISI; i++)
	{
		m_ImpData[i].Initialize();
	}
	quest::CQuestManager::instance().RequestSetEventFlag("imp_savasi", 0);
}

void CImparatorlukSavasi::OldurenKiralligaKillSay(BYTE id)
{
	m_ImpData[id].KillSay();
}

void CImparatorlukSavasi::KazananiDuyur(BYTE kazananID)
{
	const char * kazanan = "";
	if (kazananID == 1)
		kazanan = LC_TEXT("kirmizi");
	else if (kazananID == 2)
		kazanan = LC_TEXT("sari");
	else if (kazananID == 3)
		kazanan = "Mavi Bayrak";
	else 
		kazanan = kazanan;

#ifdef ENABLE_AUTO_NOTICE_SYSTEM
	CAutoNotice::instance().SendNoticeLine(1,IMP_TEXT_MAP(3),kazanan);
#else
	char szNotice[512]; char szKazanan[18];
	snprintf(szNotice, sizeof(szNotice), IMP_TEXT_MAP(3), szKazanan);
	BroadcastNotice(szNotice,true);
#endif
	TimerBaslat(this);//30 saniye sonra alayiniz gg
}

void CImparatorlukSavasi::IceriGirdimKnkBen(LPCHARACTER oyuncu)
{
	if (!oyuncu)
		return;

	m_ImpData[oyuncu->GetEmpire() - 1].OyuncuEkle();
	oyuncu->StartImpSavasDataEvent();
}

void CImparatorlukSavasi::KatilimciSayisiDusur(BYTE krallik)
{
	m_ImpData[krallik - 1].dwKatilimci -= 1;
}

DWORD CImparatorlukSavasi::GetKatilimciSayisi(BYTE krallik)
{
	return m_ImpData[krallik-1].dwKatilimci;
}

void CImparatorlukSavasi::EventBittiYallahKoye()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(MAP_INDEX);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", MAP_INDEX);
		return;
	}

	FCikDisariCik f;
	pMap->for_each(f);
	VerileriTemizle();// clear
}

void CImparatorlukSavasi::VerileriGonderKnk(LPCHARACTER oyuncu)
{ 
	if(!oyuncu || !oyuncu->GetDesc())
		return;

	oyuncu->ChatPacket(CHAT_TYPE_COMMAND, "ImpSavasBilgi %d %d %d %d %d %d %d" ,
	m_ImpData[0].dwPuan,
	m_ImpData[0].dwKatilimci,
	m_ImpData[1].dwPuan,
	m_ImpData[1].dwKatilimci,
	m_ImpData[2].dwPuan,
	m_ImpData[2].dwKatilimci,
	GetMaxKatilim() );

}
#endif