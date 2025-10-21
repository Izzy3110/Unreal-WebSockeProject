/**
 * InstaMATButton.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATButton.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "Widgets/Input/SButton.h"

/**
 * The SInstaMATButton class provides a button 
 * with the InstaMAT Style applied.
 */
class SInstaMATButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SInstaMATButton) {}
		SLATE_ATTRIBUTE(FString, Text)
		SLATE_EVENT(FOnClicked, OnClicked)
	SLATE_END_ARGS()
 
	SInstaMATButton();
	~SInstaMATButton();

	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);
};
