#include "DateTimeFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "DateTimeFunctionLibrary"

FText UDateTimeFunctionLibrary::GetTodayWeekday()
{
	switch (FDateTime::Now().GetDayOfWeek())
	{
	case EDayOfWeek::Monday:
		return LOCTEXT("Monday", "星期一");
	case EDayOfWeek::Tuesday:
		return LOCTEXT("Tuesday", "星期二");
	case EDayOfWeek::Wednesday:
		return LOCTEXT("Wednesday", "星期三");
	case EDayOfWeek::Thursday:
		return LOCTEXT("Thursday", "星期四");
	case EDayOfWeek::Friday:
		return LOCTEXT("Friday", "星期五");
	case EDayOfWeek::Saturday:
		return LOCTEXT("Saturday", "星期六");
	case EDayOfWeek::Sunday:
		return LOCTEXT("Sunday", "星期日");
	default:
		return FText::GetEmpty();
	}
}

#undef LOCTEXT_NAMESPACE
