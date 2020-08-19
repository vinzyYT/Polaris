#pragma once

// Fortnite (1.8) SDK

#ifdef _MSC_VER
	#pragma pack(push, 0x8)
#endif

#include "../SDK.hpp"

namespace SDK
{
//---------------------------------------------------------------------------
//Parameters
//---------------------------------------------------------------------------

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.Center On Widget
struct UOptionsMenuSlider_Light_C_Center_On_Widget_Params
{
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.Update Slider
struct UOptionsMenuSlider_Light_C_Update_Slider_Params
{
	struct FText                                       Slider_Text;                                              // (Parm)
	float                                              Slider_Value;                                             // (Parm, ZeroConstructor, IsPlainOldData)
	struct FText                                       Hover_Text;                                               // (Parm)
	class UCommonTextBlock*                            Tooltip_Text_Block;                                       // (Parm, ZeroConstructor, IsPlainOldData)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.Construct
struct UOptionsMenuSlider_Light_C_Construct_Params
{
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.OnMouseLeave
struct UOptionsMenuSlider_Light_C_OnMouseLeave_Params
{
	struct FPointerEvent*                              MouseEvent;                                               // (ConstParm, Parm, OutParm, ReferenceParm)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.OnMouseEnter
struct UOptionsMenuSlider_Light_C_OnMouseEnter_Params
{
	struct FGeometry*                                  MyGeometry;                                               // (Parm, IsPlainOldData)
	struct FPointerEvent*                              MouseEvent;                                               // (ConstParm, Parm, OutParm, ReferenceParm)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.BndEvt__MenuSlider_K2Node_ComponentBoundEvent_86_OnMouseCaptureEndEvent__DelegateSignature
struct UOptionsMenuSlider_Light_C_BndEvt__MenuSlider_K2Node_ComponentBoundEvent_86_OnMouseCaptureEndEvent__DelegateSignature_Params
{
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.BndEvt__MenuSlider_K2Node_ComponentBoundEvent_107_OnControllerCaptureEndEvent__DelegateSignature
struct UOptionsMenuSlider_Light_C_BndEvt__MenuSlider_K2Node_ComponentBoundEvent_107_OnControllerCaptureEndEvent__DelegateSignature_Params
{
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.BndEvt__MenuSlider_K2Node_ComponentBoundEvent_124_OnFloatValueChangedEvent__DelegateSignature
struct UOptionsMenuSlider_Light_C_BndEvt__MenuSlider_K2Node_ComponentBoundEvent_124_OnFloatValueChangedEvent__DelegateSignature_Params
{
	float                                              Value;                                                    // (Parm, ZeroConstructor, IsPlainOldData)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.BndEvt__MenuSlider_K2Node_ComponentBoundEvent_9_OnFloatValueChangedEvent__DelegateSignature
struct UOptionsMenuSlider_Light_C_BndEvt__MenuSlider_K2Node_ComponentBoundEvent_9_OnFloatValueChangedEvent__DelegateSignature_Params
{
	float                                              Value;                                                    // (Parm, ZeroConstructor, IsPlainOldData)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.ExecuteUbergraph_OptionsMenuSlider_Light
struct UOptionsMenuSlider_Light_C_ExecuteUbergraph_OptionsMenuSlider_Light_Params
{
	int                                                EntryPoint;                                               // (Parm, ZeroConstructor, IsPlainOldData)
};

// Function OptionsMenuSlider_Light.OptionsMenuSlider_Light_C.SliderChanged__DelegateSignature
struct UOptionsMenuSlider_Light_C_SliderChanged__DelegateSignature_Params
{
	float                                              Slider_Value;                                             // (Parm, ZeroConstructor, IsPlainOldData)
};

}

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
