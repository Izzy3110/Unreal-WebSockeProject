/**
 * IInstaMATVectorIntegerBaseInterface.h (InstaMAT)
 *
 * Copyright 2019-2022 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file IInstaMATVectorIntegerBaseInterface.h
 * @copyright 2019-2022 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "IInstaMATVectorIntegerBaseInterface.generated.h"

/**
 * The Vector access interface provides functions to access values of vector
 * types through Blueprint and python.
 */
UINTERFACE(MinimalAPI, NotBlueprintable)
class UInstaMATVectorIntegerBaseInterface : public UInterface
{
	GENERATED_BODY()
};
class IInstaMATVectorIntegerBaseInterface
{
	GENERATED_BODY()

public:

	/**
	 * Sets the \p Value at the specified \p Index.
	 * 
	 * @param Index the index.
	 * @param InValue the value.
	 * @return true upon success.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool SetValueAtIndex(const int32 Index, const int32 InValue) = 0;

	/**
	 * Gets the \p OutValue at the specified \p Index.
	 *
	 * @param Index the index.
	 * @param [out] OztValue the value.
	 * @return true upon success.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool GetValueAtIndex(const int32 Index, int32& OutValue) = 0;

	/**
	 * Gets the vector size for value access.
	 *
	 * @return the vector size.
	 */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetVectorSize() = 0;
};
