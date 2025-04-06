#if !defined(SINGLETON_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define SINGLETON_H

class ISingleton
{
protected:
	ISingleton() = default;
	virtual ~ISingleton() = default;

public:
	ISingleton(const ISingleton &) = delete;
	ISingleton &operator=(const ISingleton &) = delete;
};

#endif
