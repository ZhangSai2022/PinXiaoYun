#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ComponentDescriptionData.generated.h"

/** The three-part introduction shown for a component in the product data table. */
USTRUCT(BlueprintType)
struct DEMO2_API FComponentIntroduction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件介绍", meta = (DisplayName = "一级标题内容"))
	FText PrimaryTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件介绍", meta = (DisplayName = "二级标题内容"))
	FText SecondaryTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件介绍", meta = (DisplayName = "文本内容", MultiLine = "true"))
	FText BodyText;
};

/** One component row used by the component description DataTable. */
USTRUCT(BlueprintType)
struct DEMO2_API FComponentDescriptionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件说明", meta = (DisplayName = "ID"))
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件说明", meta = (DisplayName = "名称"))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件说明", meta = (DisplayName = "组件"))
	FText Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "组件说明", meta = (DisplayName = "组件介绍"))
	FComponentIntroduction Introduction;
};
