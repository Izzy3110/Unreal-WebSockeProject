/**
 * InstaMATPluginStyle.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATPluginStyle.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#ifndef InstaMAT_InstaMATPluginStyle_h
#define InstaMAT_InstaMATPluginStyle_h

#include "CoreMinimal.h"
#include "SlateFwd.h"

#include "Styling/SlateStyle.h"

/**
 * The FInstaMATPluginStyle class contains UI styles.
 */
class INSTAMAT_API FInstaMATPluginStyle
{
public:

	/**
	 * Initializes down the style.
	 */
	static void Initialize();

	/**
	 * Shuts down the style.
	 */
	static void Shutdown();
	
	/** 
	 * Reloads textures used by slate renderer.
	 */
	static void ReloadTextures();
	
	/**
	 * Gets the Slate Style instance.
	 *
	 * @return the instance.
	 */
	static const ISlateStyle& Get();

	/**
	 * Gets the Style Set name.
	 *
	 * @return the name.
	 */
	static FName GetStyleSetName();
	
	/**
	 * Gets the InstaMAT color.
	 *
	 * @return The InstaMAT color.
	 */
	static FColor GetInstaMATBlue()
	{
		return FColor(1, 129, 159);
	}

private:

	/**
	 * Creates a shared reference.
	 *
	 * @return the reference.
	 */
	static TSharedRef<class FSlateStyleSet> Create();
	
private:
	
	static TSharedPtr<class FSlateStyleSet> StyleInstance;	/**< The static instance. */
};


#endif
