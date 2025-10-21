/**
 * InstaMATInputBase.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATInputBase.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATImporterGraphInstance.h"
#include "IInstaMATVectorFloatBaseInterface.h"
#include "IInstaMATVectorIntegerBaseInterface.h"
#include "InstaMATInputBase.generated.h"

/**
 * The UInstaMATInputBase is the base class for all input objects.
 */
UCLASS(Abstract, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputBase : public UObject
{
	GENERATED_BODY()
public:

	/** The name of the input. */
	UPROPERTY(BlueprintReadOnly)
	FString InputName;

	/** The Parent graph id of the InstaMAT GraphObject. */
	UPROPERTY(BlueprintReadOnly)
	FString ParentGraphID;

	/** The category for this input. */
	UPROPERTY(BlueprintReadOnly)
	FString Category;

	/** The parameter index in the InstaMAT GraphObject. */
	UPROPERTY(BlueprintReadOnly)
	int32 Index;

	/** The tooltip text of this input. */
	UPROPERTY(BlueprintReadOnly)
	FString ToolTip;

	/** The parent GraphInstance of this object. */
	UPROPERTY(BlueprintReadOnly)
	UInstaMATImporterGraphInstance* Parent;

	/** Determines whether the current UI element shall be visible. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsVisible = true;

	/** The control type provided by InstaMAT. */
	UPROPERTY(BlueprintReadOnly)
	int32 ControlType = 0;

	/**
	 * The PostEditChangeProperty is called after a property is changed.
	 * 
	 * @param PropertyChangedEvent the property changed event info object.
	 */
	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
			return;

		UObject::PostEditChangeProperty(PropertyChangedEvent);

		// NOTE: while force deleting the parent may be null and Unreal Engine invokes a property change
		if (Parent != nullptr)
		{
			Parent->SetDirty(true);
		}
	}

	/**
	 * Determines whether the current value is matching the default value.
	 *
	 * @return true if default value.
	 */
	virtual FORCEINLINE bool IsDefaultValue() { return true; };

	/**
	 * Resets the value to default. And sets the Parent Object dirty.
	 */
	virtual FORCEINLINE void Reset() { Parent->SetDirty(true); };
};

/**
 * The UInstaMATInputBoolean class handles boolean input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputBoolean : public UInstaMATInputBase
{
	GENERATED_BODY()
public:

	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Value; 

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	bool DefaultValue;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}
};

/**
 * The UInstaMATInputFloat32 class handles float input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputFloat32 : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value; 

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	float DefaultValue; 

	/** The minimum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	float MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	float MaximumValue;

	/** Determines whether the value is clamped in value range. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			Value = FMath::Clamp(Value, MinimumValue, MaximumValue);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	/** Determines whether the corresponding UI will be visible in the panel. */
	bool bIsVisible = true; 
};

/**
 * The UInstaMATInputInt32 class handles integer input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputInt32 : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value; 

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	int32 DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	int32 MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	int32 MaximumValue; 

	/** Determines whether this instance is clamped in value range. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	/** Determines whether this instance holds unsigned values. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnsigned;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsUnsigned)
		{
			Value = FMath::Max(Value, 0); 
		}
		if (bIsRangeLimited)
		{
			Value = FMath::Clamp(Value, MinimumValue, MaximumValue);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}
};

/**
 * The UInstaMATInputVector2F class handles vector2 float input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector2F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:
	static const int32 VectorSize = 2; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	FVector2f Value;

	/** The default value of this instance. */
	UPROPERTY()
	FVector2f DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY()
	FVector2f MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY()
	FVector2f MaximumValue;

	/** Determines whether the instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}
	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			Value.X = FMath::Clamp(Value.X, MinimumValue.X, MaximumValue.X);
			Value.Y = FMath::Clamp(Value.Y, MinimumValue.Y, MaximumValue.Y);
		}

		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector2F::VectorSize)
			return false;

		Value[ValueIndex] = FMath::Clamp(InValue, MinimumValue[ValueIndex], MaximumValue[ValueIndex]);

		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputVector2F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector2F::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}
};

/**
 * The UInstaMATInputVector3F class handles vector3 float input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector3F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 VectorSize = 3; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector3f Value;

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FVector3f DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FVector3f MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FVector3f MaximumValue;

	/** Determines whether the instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			Value.X = FMath::Clamp(Value.X, MinimumValue.X, MaximumValue.X);
			Value.Y = FMath::Clamp(Value.Y, MinimumValue.Y, MaximumValue.Y);
			Value.Z = FMath::Clamp(Value.Z, MinimumValue.Z, MaximumValue.Z);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector3F::VectorSize)
			return false;

		Value[ValueIndex] = FMath::Clamp(InValue, MinimumValue[ValueIndex], MaximumValue[ValueIndex]);

		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputVector3F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector3F::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}
}; 

/**
 * The UInstaMATInputVector4F class handles vector3 float input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector4F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:
	static const int32 VectorSize = 4; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	FVector4f Value;

	/** The default value of this instance. */
	UPROPERTY()
	FVector4f DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY()
	FVector4f MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY()
	FVector4f MaximumValue;

	/** Determines whether the instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			Value.X = FMath::Clamp(Value.X, MinimumValue.X, MaximumValue.X);
			Value.Y = FMath::Clamp(Value.Y, MinimumValue.Y, MaximumValue.Y);
			Value.Z = FMath::Clamp(Value.Z, MinimumValue.Z, MaximumValue.Z);
			Value.W = FMath::Clamp(Value.W, MinimumValue.W, MaximumValue.W);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector4F::VectorSize)
			return false;

		Value[ValueIndex] = FMath::Clamp(InValue, MinimumValue[ValueIndex], MaximumValue[ValueIndex]);

		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputVector4F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector4F::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}
};

/**
 * The UInstaMATInputVector2I32 class handles vector2 integer input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector2I32 : public UInstaMATInputBase, public IInstaMATVectorIntegerBaseInterface
{
	GENERATED_BODY()
public:
	static const int32 VectorSize = 2; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Value;

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntPoint DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntPoint MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntPoint MaximumValue;

	/** Determines whether the instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	/** Determines whether the instance is unsigned. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnsigned;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsUnsigned)
		{
			Value.X = FMath::Max(Value.X, 0);
			Value.Y = FMath::Max(Value.Y, 0);
		}
		if (bIsRangeLimited)
		{
			Value.X = FMath::Clamp(Value.X, MinimumValue.X, MaximumValue.X);
			Value.Y = FMath::Clamp(Value.Y, MinimumValue.Y, MaximumValue.Y);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const int32 InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector2I32::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, int32& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector2I32::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize()
	{
		return UInstaMATInputVector2I32::VectorSize;
	}
};

/**
 * The UInstaMATInputVector3I32 class handles vector3 integer input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector3I32 : public UInstaMATInputBase, public IInstaMATVectorIntegerBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 VectorSize = 3; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Value;

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntVector DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntVector MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FIntVector MaximumValue;

	/** Determines whether the value is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	/** Determines whether the value is unsigned. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnsigned;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsUnsigned)
		{
			Value.X = FMath::Max(Value.X, 0);
			Value.Y = FMath::Max(Value.Y, 0);
			Value.Z = FMath::Max(Value.Z, 0);
		}
		if (bIsRangeLimited)
		{
			Value.X = FMath::Clamp(Value.X, MinimumValue.X, MaximumValue.X);
			Value.Y = FMath::Clamp(Value.Y, MinimumValue.Y, MaximumValue.Y);
			Value.Z = FMath::Clamp(Value.Z, MinimumValue.Z, MaximumValue.Z);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const int32 InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector3I32::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, int32& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector3I32::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize()
	{
		return UInstaMATInputVector3I32::VectorSize;
	}
};

/**
 * The UInstaMATInputVector4I32 class handles vector4 integer input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputVector4I32 : public UInstaMATInputBase, public IInstaMATVectorIntegerBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 VectorSize = 4; /**< The number of elements in this Vector. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	int32 Value[UInstaMATInputVector4I32::VectorSize];

	/** The default value of this instance. */
	UPROPERTY()
	int32 DefaultValue[UInstaMATInputVector4I32::VectorSize];

	/** The minimum valid value of this instance. */
	UPROPERTY()
	int32 MinimumValue[UInstaMATInputVector4I32::VectorSize];

	/** The maximum valid value of this instance. */
	UPROPERTY()
	int32 MaximumValue[UInstaMATInputVector4I32::VectorSize];

	/** Determines whether this instance is range limitied. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	/** Determines whether the value is unsigned. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnsigned;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value[0] = DefaultValue[0];
		Value[1] = DefaultValue[1];
		Value[2] = DefaultValue[2];
		Value[3] = DefaultValue[3];
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return	
			Value[0] == DefaultValue[0] &&
			Value[1] == DefaultValue[1] &&
			Value[2] == DefaultValue[2] &&
			Value[3] == DefaultValue[3];
	}
	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsUnsigned)
		{
			Value[0] = FMath::Max(Value[0], 0);
			Value[1] = FMath::Max(Value[1], 0);
			Value[2] = FMath::Max(Value[2], 0);
			Value[3] = FMath::Max(Value[3], 0);
		}
		if (bIsRangeLimited)
		{
			Value[0] = FMath::Clamp(Value[0], MinimumValue[0], MaximumValue[0]);
			Value[1] = FMath::Clamp(Value[1], MinimumValue[1], MaximumValue[1]);
			Value[2] = FMath::Clamp(Value[2], MinimumValue[2], MaximumValue[2]);
			Value[3] = FMath::Clamp(Value[3], MinimumValue[3], MaximumValue[3]);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const int32 InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector4I32::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, int32& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputVector4I32::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize()
	{
		return UInstaMATInputVector4I32::VectorSize;
	}
};

/**
 * The UInstaMATInputMatrix2F class handles Matrix 2x2 input values.
 * NOTE: the underlying data structure is a Vector4F.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputMatrix2F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 VectorSize = 4; /**< The total number of elements in this Matrix. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	FVector4f Value;

	/** The default value of this instance. */
	UPROPERTY()
	FVector4f DefaultValue;

	/** The minimum valid value of this instance. */
	UPROPERTY()
	FVector4f MinimumValue;

	/** The maximum valid value of this instance. */
	UPROPERTY()
	FVector4f MaximumValue;

	/** Determines whether this instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			Value[0] = FMath::Clamp(Value[0], MinimumValue[0], MaximumValue[0]);
			Value[1] = FMath::Clamp(Value[1], MinimumValue[1], MaximumValue[1]);
			Value[2] = FMath::Clamp(Value[2], MinimumValue[2], MaximumValue[2]);
			Value[3] = FMath::Clamp(Value[3], MinimumValue[3], MaximumValue[3]);
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix2F::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputMatrix2F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix2F::VectorSize)
			return false;

		OutValue = Value[ValueIndex];
		return true;
	}
};

/**
 * The UInstaMATInputMatrix3F class handles Matrix 3x3 input values.
 * NOTE: The underlying data structure is a float array.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputMatrix3F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 VectorSize = 9; /**< The total number of elements in this Matrix. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	float Value[UInstaMATInputMatrix3F::VectorSize];

	/** The default value of this instance. */
	UPROPERTY()
	float DefaultValue[UInstaMATInputMatrix3F::VectorSize];

	/** The minimum valid value of this instance. */
	UPROPERTY()
	float MinimumValue[UInstaMATInputMatrix3F::VectorSize];

	/** The maximum valid value of this instance. */
	UPROPERTY()
	float MaximumValue[UInstaMATInputMatrix3F::VectorSize];

	/** Determines whether this instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix3F::VectorSize; ValueIndex++)
			{
				Value[ValueIndex] = FMath::Clamp(Value[ValueIndex], MinimumValue[ValueIndex], MaximumValue[ValueIndex]);
			}
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix3F::VectorSize; ValueIndex++)
		{
			Value[ValueIndex] = DefaultValue[ValueIndex];
		}
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix3F::VectorSize; ValueIndex++)
		{
			if (Value[ValueIndex] != DefaultValue[ValueIndex])
				return false;
		}

		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix3F::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputMatrix3F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix3F::VectorSize)
			return false;
		
		OutValue = Value[ValueIndex];
		return true;
	}
};

/**
 * The UInstaMATInputMatrix4F class handles Matrix 4x4 input values.
 * NOTE: The underlying data structure is a float array.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputMatrix4F : public UInstaMATInputBase, public IInstaMATVectorFloatBaseInterface
{
	GENERATED_BODY()
public:

	static const int32 ColumnSize = 4;							/**< The number of columns in this Matrix. */
	static const int32 VectorSize = ColumnSize * ColumnSize;	/**< The total number of elements in this Matrix. */

	/** The value of this instance. */
	UPROPERTY(EditAnywhere)
	float Value[UInstaMATInputMatrix4F::VectorSize];

	/** The default value of this instance. */
	UPROPERTY()
	float DefaultValue[UInstaMATInputMatrix4F::VectorSize];

	/** The minimum valid value of this instance. */
	UPROPERTY()
	float MinimumValue[UInstaMATInputMatrix4F::VectorSize];

	/** The maximum valid value of this instance. */
	UPROPERTY()
	float MaximumValue[UInstaMATInputMatrix4F::VectorSize];

	/** Determines whether this instance is range limited. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRangeLimited;

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (bIsRangeLimited)
		{
			for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix4F::VectorSize; ValueIndex++)
			{
				Value[ValueIndex] = FMath::Clamp(Value[ValueIndex], MinimumValue[ValueIndex], MaximumValue[ValueIndex]);
			}
		}
		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix4F::VectorSize; ValueIndex++)
		{
			Value[ValueIndex] = DefaultValue[ValueIndex];
		}
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		for (int ValueIndex = 0; ValueIndex < UInstaMATInputMatrix4F::VectorSize; ValueIndex++)
		{
			if (Value[ValueIndex] != DefaultValue[ValueIndex])
				return false;
		}
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool SetValueAtIndex(const int32 ValueIndex, const float InValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix4F::VectorSize)
			return false;

		Value[ValueIndex] = InValue;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE int32 GetVectorSize() override
	{
		return UInstaMATInputMatrix4F::VectorSize;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool GetValueAtIndex(const int32 ValueIndex, float& OutValue) override
	{
		if (ValueIndex < 0 || ValueIndex >= UInstaMATInputMatrix4F::VectorSize)
			return false;
		
		OutValue = Value[ValueIndex];
		return true;
	}
};

/**
 * The UInstaMATInputElementImage class handles color and image input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputElementImage : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The Color value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor ColorValue;

	/** The default color value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FColor DefaultColorValue;

	/** The texture value of this instance. */
	UPROPERTY(EditAnywhere)
	class UTexture2D* TextureValue;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		TextureValue = nullptr;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return TextureValue == nullptr;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDefaultColorValue()
	{
		return ColorValue == DefaultColorValue;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ResetColor()
	{
		Super::Reset();
		ColorValue = DefaultColorValue;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsColorValueEnabled() const
	{
		return TextureValue == nullptr;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsTextureSet() const
	{
		return TextureValue != nullptr;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		// Dragging slider, changing color
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
			return;

		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);

		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UInstaMATInputElementImage, TextureValue))
		{
			bGraphInputRequiresReset = TextureValue == nullptr;
		}
	}

	UPROPERTY()
	bool bGraphInputRequiresReset = false;
};

/**
 * The UInstaMATInputElementImageGrayscale class handles grayscale and image input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputElementImageGrayscale : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The grayscale value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrayscaleValue;

	/** The default grayscale value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	float DefaultGrayscaleValue;

	/** The texture value of this instance. */
	UPROPERTY(EditAnywhere)
	class UTexture2D* TextureValue;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		TextureValue = nullptr;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return TextureValue == nullptr;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDefaultGrayscaleValue()
	{
		return GrayscaleValue == DefaultGrayscaleValue;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ResetGrayscaleValue()
	{
		Super::Reset();
		GrayscaleValue = DefaultGrayscaleValue;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsGrayscaleValueEnabled() const
	{
		return TextureValue == nullptr;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsTextureSet() const
	{
		return TextureValue != nullptr;
	}

	virtual FORCEINLINE void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		// Dragging slider, changing color
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
			return;

		UInstaMATInputBase::PostEditChangeProperty(PropertyChangedEvent);

		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UInstaMATInputElementImageGrayscale, TextureValue))
		{
			bGraphInputRequiresReset = TextureValue == nullptr;
		}
	}

	UPROPERTY()
	bool bGraphInputRequiresReset = false;
};

/**
 * The UInstaMATInputElementString class handles string input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputElementString : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The string value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FString DefaultValue;

	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}
};

/**
 * The UInstaMATInputEnumValue class handles enum input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputEnumValue : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Value;

	/** The default value of this instance. */
	UPROPERTY(BlueprintReadOnly)
	int32 DefaultValue;

	/** The Enum string values. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> EnumValues;

	/** The Enum string values. */
	TArray<TSharedPtr<FString>> EnumStringValues;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool SetValue(const int32 ValueIndex)
	{
		if (ValueIndex < 0 || ValueIndex >= EnumValues.Num())
			return false;
		
		Value = ValueIndex;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetCurrentEnumValueAsString()
	{
		return EnumValues[Value];
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = DefaultValue;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == DefaultValue;
	}
};

/**
 * The UInstaMATInputElementMesh class handles mesh input values.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputElementMesh : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	/** The mesh value of this instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMesh* Value;

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void Reset() override
	{
		Super::Reset();
		Value = nullptr;
	}

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override
	{
		return Value == nullptr;
	}
};

/**
 * Generic Input class to handle inputs that are not supported in this integration.
 * 
 * This class is intentionally empty and it does not contain any 'Value'. It uses a custom detail builder 
 * to let the user know the input is not supported on Unreal.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATInputNotSupported : public UInstaMATInputBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool IsDefaultValue() override { return true; }
};