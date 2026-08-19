#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DateTimeFunctionLibrary.generated.h"

UCLASS()
class DEMO2_API UDateTimeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the local system date's weekday in Chinese, for example "星期一". */
	UFUNCTION(BlueprintPure, Category = "DEMO2|日期时间", meta = (DisplayName = "获取当天星期", Keywords = "星期 weekday today"))
	static FText GetTodayWeekday();
};
