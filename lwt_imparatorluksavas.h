#ifdef ENABLE_IMPARATORLUK_SAVASI
class CImparatorlukSavasi : public singleton<CImparatorlukSavasi>
{

	typedef struct SImpTable
	{
		BYTE	bNo;
		DWORD	dwPuan;
		DWORD	dwKatilimci;

		void	Initialize();

		void	KillSay();
		void	OyuncuEkle();
		void	OyuncuSil();
	}TImpTable;
	public:
		enum bilgiler
		{
			KRALLIK_SAYISI = 3,
			MAP_INDEX = 187,
		};

		CImparatorlukSavasi() { VerileriTemizle(); };
		~CImparatorlukSavasi() { VerileriTemizle(); };

		void		VerileriTemizle();
		void		EventBaslat();
		void		EventBittiYallahKoye();
		void		OldurenKiralligaKillSay(BYTE id);
		void		KazananiDuyur(BYTE kazananID);
		void		IceriGirdimKnkBen(LPCHARACTER oyuncu);
		void		KatilimciSayisiDusur(BYTE krallik);
		void		VerileriGonderKnk(LPCHARACTER oyuncu);

/*** Set - Get Metodlari ***/
		void		SetMaxSkor(DWORD skor) { MaxSkor = skor; }
		DWORD		GetMaxSkor() { return MaxSkor; }

		void		SetMaxLevel(BYTE level) { GirisLevel = level; }
		BYTE		GetMaxLevel() { return GirisLevel; }

		void		SetMaxKatilim(DWORD gelen) { MaxKatilim = gelen; }
		DWORD		GetMaxKatilim() { return MaxKatilim; }

		DWORD GetKatilimciSayisi(BYTE krallik);
/*** Set - Get Metodlari ***/

	private:
		BYTE KazananKrallik = 0; BYTE GirisLevel = 90; DWORD MaxSkor = 3; DWORD MaxKatilim = 50;
		TImpTable	m_ImpData[3];
};
#endif