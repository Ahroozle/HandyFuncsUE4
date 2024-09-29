// Fill out your copyright notice in the Description page of Project Settings.

#include "HandyFuncs.h"

// unsure which ones of these are needed from the files so I'm just including all of them. Sorry :(

#include "Sound/SoundCue.h"

#include "SkeletalSpriteAnimation.h"

#include "Kismet/GameplayStatics.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Slate/SceneViewport.h"
#include "Components/Button.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/LocalPlayer.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

#include "Components/StaticMeshComponent.h"


void UHandyFuncs::MakeQBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, int NumPoints, TArray<FVector>& OutPoints)
{
	//// Naive Impl, ~1100 ops per 100 points
	//
	//const float dT = 1.0f / (NumPoints - 1); // change in time

	//float T = 0; // currTime

	//// nothin to see here officer just a trainwreck :^)
	//float iT; // Inverse time variable
	//for (int currStep = 0; currStep < NumPoints; ++currStep)
	//{
	//	iT = 1 - T;

	//	// B(t) = ((1-t)^2 * P0) + (2t * (1-t) * P1) + (t^2 * P2)
	//	OutPoints.Add(((iT*iT)*P0) + (2*T*iT*P1) + (T*T*P2));

	//	T += dT;
	//}


	// Forward Difference impl, ~300 ops per 100 points

	const float dT = 1.0f / (NumPoints - 1); // change in time

											 // Coefficients for the polynomial, gotten by plugging the equation ( ((1-t)^2 * P0) + (2t * (1-t) * P1) + (t^2 * P2) )
											 // into Wolfram Alpha's Taylor Series Calculator.
	const FVector a = P0;
	const FVector b = 2 * (P1 - P0);
	const FVector c = P0 - (2 * P1) + P2;

	// NOTE: for some reason you apply the coeffs in backwards order
	// i.e. instead of the formula being Ax^2 + Bx + C
	// it is Cx^2 + Bx + A
	//
	// This was explained literally nowhere so I just felt like I should write it down here
	// so I don't flop around in agony for 2 days straight again
	FVector S = a;					// P(t) @ t=0
	FVector U = c*dT*dT + b*dT;		// First Diff of P(t) (equ. 2aht + ah^2 + bh) @ t=0
	FVector V = 2 * c*dT*dT;		// Second Diff of P(t) (equ 2ah^2) @t=0

	OutPoints.Add(P0);
	for (int currStep = 1; currStep < NumPoints; ++currStep)
	{
		S += U;
		U += V;

		OutPoints.Add(S);
	}

}



TSharedPtr<FGenericWindow> UHandyFuncs::GetMainNativeWindow()
{
	GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

	if (nullptr != GEngine->GameViewport)
	{
		TSharedPtr<SWindow> RetrievedWind = GEngine->GameViewport->GetWindow();

		if (RetrievedWind.IsValid())
			return RetrievedWind->GetNativeWindow();
	}

	return nullptr;
}

TSharedPtr<FGenericWindow> UHandyFuncs::GetNativeSubWindow(UWidget* SubWindowContext)
{
	if (nullptr != SubWindowContext)
	{
		TSharedPtr<SWindow> RetrievedWind = FSlateApplication::Get().FindWidgetWindow(SubWindowContext->TakeWidget());

		if (RetrievedWind.IsValid())
			return RetrievedWind->GetNativeWindow();
	}

	return nullptr;
}

bool UHandyFuncs::OpenFileDialog(
	const FString& DialogTitle,
	const FString& DefaultPath,
	const FString& DefaultFile,
	const FString& FileTypes,
	bool MultipleFiles,
	TArray<FString>& OutFilenames,
	UWidget* SubWindowContext
)
{
	TSharedPtr<FGenericWindow> NativeWind = GetMainNativeWindow();

	if (!NativeWind.IsValid())
		NativeWind = GetNativeSubWindow(SubWindowContext);

	if (NativeWind.IsValid())
	{
		const void* OSWind = NativeWind->GetOSWindowHandle();

		return FDesktopPlatformModule::Get()->OpenFileDialog(OSWind, DialogTitle, DefaultPath, DefaultFile, FileTypes, (MultipleFiles ? 0x0 : 0x1), OutFilenames);
	}

	return false;
}

bool UHandyFuncs::SaveFileDialog(
	const FString& DialogTitle,
	const FString& DefaultPath,
	const FString& DefaultFile,
	const FString& FileTypes,
	bool MultipleFiles,
	TArray<FString>& OutFilenames,
	UWidget* SubWindowContext
)
{
	TSharedPtr<FGenericWindow> NativeWind = GetMainNativeWindow();

	if (!NativeWind.IsValid())
		NativeWind = GetNativeSubWindow(SubWindowContext);

	if (NativeWind.IsValid())
	{
		const void* OSWind = NativeWind->GetOSWindowHandle();

		return FDesktopPlatformModule::Get()->SaveFileDialog(OSWind, DialogTitle, DefaultPath, DefaultFile, FileTypes, (MultipleFiles ? 0x0 : 0x1), OutFilenames);
	}

	return false;
}

bool UHandyFuncs::GetStringFromFile(FString& OutString, const FString& FilePath)
{
	return FFileHelper::LoadFileToString(OutString, *FilePath);
}

bool UHandyFuncs::SaveStringToFile(const FString& StringToSave, const FString& FilePath)
{
	return FFileHelper::SaveStringToFile(StringToSave, *FilePath);
}



void UHandyFuncs::MakeComplementaryColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	FLinearColor ComplementHSV(FMath::Fmod(Hue + 180, 360), Sat, Val);

	OutColors.Add(ComplementHSV.HSVToLinearRGB());
}

void UHandyFuncs::MakeAnalogousColorScheme(FLinearColor Root, int NumColors, float AngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (NumColors > 1)
	{
		while (--NumColors)
		{
			if (InSatVals.Num() > OutColors.Num())
			{
				Sat = InSatVals[OutColors.Num()].X;
				Val = InSatVals[OutColors.Num()].Y;
			}
			else if (InSatVals.Num() > 0)
			{
				Sat = InSatVals.Last().X;
				Val = InSatVals.Last().Y;
			}

			Hue = FMath::Fmod(Hue + AngleHSV, 360);
			OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());
		}
	}
}

void UHandyFuncs::MakeTriadicColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 120, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 240, 360), Sat, Val).HSVToLinearRGB());
}

void UHandyFuncs::MakeSplitComplementaryColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

}

void UHandyFuncs::MakeTetradicColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 3)
	{
		Sat = InSatVals[3].X;
		Val = InSatVals[3].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());
}

void UHandyFuncs::MakeSquareColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 90, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 3)
	{
		Sat = InSatVals[3].X;
		Val = InSatVals[3].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 270, 360), Sat, Val).HSVToLinearRGB());
}

void UHandyFuncs::MakeMonochromeColorScheme(FLinearColor Root, int NumColors, FVector2D IntervalRange, TArray<FLinearColor>& OutColors)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = RootHSV.G;
	float Val = RootHSV.B + FMath::FRandRange(IntervalRange.X, IntervalRange.Y);

	OutColors.Add(Root);

	if (NumColors > 1)
	{
		while (--NumColors)
		{
			OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());
			Val += FMath::FRandRange(IntervalRange.X, IntervalRange.Y);
		}
	}
}

UTexture2D* UHandyFuncs::MakeTextureForColorScheme(const TArray<FLinearColor>& Colors)
{
	UTexture2D* NewTex = UTexture2D::CreateTransient(Colors.Num(), 1);

	NewTex->Filter = TextureFilter::TF_Nearest;

	auto& MipZero = NewTex->PlatformData->Mips[0];

	FColor* MipData = static_cast<FColor*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

	for (int i = 0; i < Colors.Num(); ++i)
		MipData[i] = Colors[i].ToFColor(true);

	MipZero.BulkData.Unlock();
	NewTex->UpdateResource();

	return NewTex;
}



UTexture2D* UHandyFuncs::MakeTextureFromRenderTarget(UTextureRenderTarget2D* Target, TArray<FLinearColor>& OutColors, bool InvertOpacity)
{
	if (nullptr != Target)
	{
		UTexture2D* ConstructedTex = Target->ConstructTexture2D((UObject*)GetTransientPackage(), "NewRT_Tex", EObjectFlags::RF_NoFlags);

		auto& MipZero = ConstructedTex->PlatformData->Mips[0];

		if (ConstructedTex->Source.GetFormat() == TSF_BGRA8)
		{
			FColor* MipData = static_cast<FColor*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

			int TotalCells = (ConstructedTex->GetSizeY() - 1) * ConstructedTex->GetSurfaceWidth() + (ConstructedTex->GetSizeX() - 1);

			for (int i = 0; i <= TotalCells; ++i)
			{
				if (InvertOpacity)
					MipData[i].A = 255 - MipData[i].A;

				OutColors.Add(MipData[i]);
			}
		}
		else if (ConstructedTex->Source.GetFormat() == TSF_RGBA16F)
		{
			FFloat16Color* MipData = static_cast<FFloat16Color*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

			int TotalCells = (ConstructedTex->GetSizeY() - 1) * ConstructedTex->GetSurfaceWidth() + (ConstructedTex->GetSizeX() - 1);

			for (int i = 0; i <= TotalCells; ++i)
			{
				if (InvertOpacity)
					MipData[i].A = 1.0f - MipData[i].A;

				OutColors.Add({ MipData[i].R,MipData[i].G,MipData[i].B,MipData[i].A });
			}
		}

		if (MipZero.BulkData.IsLocked())
		{
			MipZero.BulkData.Unlock();
			ConstructedTex->UpdateResource();
		}

		return ConstructedTex;
	}

	return nullptr;
}



void UHandyFuncs::ConstructVectorFieldFromUVs(FIntVector Res, FBox LocalBounds, UTexture2D* LocalPositions, UTexture2D* VectorValues, FString FilePath)
{
	if (nullptr != LocalPositions && nullptr != VectorValues &&
		LocalPositions->GetSizeX() == VectorValues->GetSizeX() &&
		LocalPositions->GetSizeY() == VectorValues->GetSizeY() &&
		LocalPositions->Source.GetFormat() == VectorValues->Source.GetFormat())
	{
		TArray<FVector4> Values;
		Values.AddDefaulted(Res.X * Res.Y * Res.Z);

		FVector BoundsWidth = LocalBounds.Max - LocalBounds.Min;

		{
			auto TryWriteValue = [&Values, &BoundsWidth, &LocalBounds, &Res](const FLinearColor& LocalPosCol, const FLinearColor& VectorValCol)
			{
				if (!LocalPosCol.Equals(FLinearColor::Black))
				{
					FVector LocPosVec(LocalPosCol);
					FVector ValueVec(VectorValCol);

					FVector InRange = (LocPosVec - LocalBounds.Min) / BoundsWidth;

					int FinalIndex =
						(FMath::Clamp(FMath::FloorToInt(InRange.Z * Res.Z), 0, Res.Z - 1) * Res.X * Res.Y) +
						(FMath::Clamp(FMath::FloorToInt(InRange.Y * Res.Y), 0, Res.Y - 1) * Res.X) +
						FMath::Clamp(FMath::FloorToInt(InRange.X * Res.X), 0, Res.X - 1);

					Values[FinalIndex] += FVector4(ValueVec, 1.0f);
				}
			};

			auto& LocPosMipZero = LocalPositions->PlatformData->Mips[0];
			auto& ValsMipZero = VectorValues->PlatformData->Mips[0];

			if (LocalPositions->Source.GetFormat() == TSF_BGRA8)
			{
				const FColor* LocPosMipData = static_cast<const FColor*>(LocPosMipZero.BulkData.Lock(LOCK_READ_ONLY));
				const FColor* ValsMipData = static_cast<const FColor*>(ValsMipZero.BulkData.Lock(LOCK_READ_ONLY));

				int TotalCells = (LocalPositions->GetSizeY() - 1) * LocalPositions->GetSurfaceWidth() + (LocalPositions->GetSizeX() - 1);

				for (int i = 0; i <= TotalCells; ++i)
					TryWriteValue(LocPosMipData[i], ValsMipData[i]);
			}
			else if (LocalPositions->Source.GetFormat() == TSF_RGBA16F)
			{
				const FFloat16Color* LocPosMipData = static_cast<const FFloat16Color*>(LocPosMipZero.BulkData.Lock(LOCK_READ_ONLY));
				const FFloat16Color* ValsMipData = static_cast<const FFloat16Color*>(ValsMipZero.BulkData.Lock(LOCK_READ_ONLY));

				int TotalCells = (LocalPositions->GetSizeY() - 1) * LocalPositions->GetSurfaceWidth() + (LocalPositions->GetSizeX() - 1);

				for (int i = 0; i <= TotalCells; ++i)
				{
					FLinearColor LocCol =
					{ LocPosMipData[i].R.GetFloat(), LocPosMipData[i].G.GetFloat(), LocPosMipData[i].B.GetFloat(), 1.0f };
					FLinearColor ValCol =
					{ ValsMipData[i].R.GetFloat(), ValsMipData[i].G.GetFloat(), ValsMipData[i].B.GetFloat(), 1.0f };
					TryWriteValue(LocCol, ValCol);
				}
			}

			if (LocPosMipZero.BulkData.IsLocked())
				LocPosMipZero.BulkData.Unlock();

			if (ValsMipZero.BulkData.IsLocked())
				ValsMipZero.BulkData.Unlock();
		}


		auto VecToCSVStr = [](const FVector& a)
		{
			return FString::SanitizeFloat(a.X) + "," + FString::SanitizeFloat(a.Y) + "," + FString::SanitizeFloat(a.Z) + ",\n";
		};

		FString FileStr =
			FString::FromInt(Res.X) + "," + FString::FromInt(Res.Y) + "," + FString::FromInt(Res.Z) + ",\n" +
			VecToCSVStr(LocalBounds.Min) + VecToCSVStr(LocalBounds.Max);

		for (FVector4& curr : Values)
		{
			FVector FinalVec = { curr.X,curr.Y,curr.Z };
			FinalVec /= curr.W;

			FileStr += VecToCSVStr(FinalVec);
		}

		FileStr.RemoveFromEnd("\n");

		SaveStringToFile(FileStr, FilePath);
	}
}



float UHandyFuncs::ConvertHorizontalToVerticalFOV(float FOV, int ScreenWidth, int ScreenHeight)
{
	return 2 * FMath::Atan(FMath::Tan(FOV / 2)*(float(ScreenHeight) / ScreenWidth));
}

float UHandyFuncs::ConvertVerticalToHorizontalFOV(float FOV, int ScreenWidth, int ScreenHeight)
{
	return 2 * FMath::Atan(FMath::Tan(FOV / 2)*(float(ScreenWidth) / ScreenHeight));
}



float UHandyFuncs::TimeToImpact_NoGravity(FVector ShooterPosition, FVector TargetPosition, FVector TargetVelocity, float ProjectileSpeed)
{
	// yoinked from http://playtechs.blogspot.com/2007/04/aiming-at-moving-target.html

	FVector ToTarg = TargetPosition - ShooterPosition;

	const float a = (ProjectileSpeed * ProjectileSpeed) - TargetVelocity.SizeSquared();
	const float b = ToTarg | TargetVelocity;
	const float c = ToTarg.SizeSquared();

	const float d = b*b + a*c;

	float t = 0;
	if (d >= 0)
	{
		const float otherroot = (b - FMath::Sqrt(d)) / a;

		// extra case for when the target is moving faster than the particle; the negative root may actually be valid!
		if (otherroot >= 0)
			t = otherroot;
		else
			t = FMath::Max((b + FMath::Sqrt(d)) / a, 0.0f);
	}

	return t;
}


bool UHandyFuncs::SolveQuadraticEquation(float A, float B, float C, TArray<float>& ValidRoots)
{
	// https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_ballistic_trajectory.cs#L48

	// Fixing NaNs
	if (FMath::IsNearlyZero(A))
		A = 0.00001;

	// shift to normal form (x^2 + px + q)
	const float p = B / (2 * A);
	const float q = C / A;

	const float D = p * p - q;

	if (FMath::IsNearlyZero(D))
		ValidRoots.Add(-p);
	else if (D > 0)
	{
		const float D_sqrt = FMath::Sqrt(D);
		ValidRoots.Add(D_sqrt - p);
		ValidRoots.Add(-D_sqrt - p);
	}

	return ValidRoots.Num() > 0;
}

bool UHandyFuncs::SolveCubicEquation(float A, float B, float C, float D, TArray<float>& ValidRoots)
{
	// https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_ballistic_trajectory.cs#L78

	// Fixing NaNs
	if (FMath::IsNearlyZero(A))
		A = 0.00001;

	// shift to normal form (x^3 + ax^2 + bx + c)
	const float a = B / A;
	const float b = C / A;
	const float c = D / A;

	// sub x = y - a/3 to remove quadratic term (x^3 + px + q)
	const float a_squared = a * a;
	const float one_third = 1.0f / 3.0f;
	const float p = one_third * (-one_third * a_squared + b);
	const float q = 0.5f * (0.074074074074f * a * a_squared - one_third * a * b + c);

	// Cardano's formula
	const float cb_p = p * p * p;
	const float d = q * q + cb_p;

	if (FMath::IsNearlyZero(d))
	{
		if (FMath::IsNearlyZero(q)) // one triple solution
			ValidRoots.Add(0);
		else // a single and a double solution
		{
			const float u = FMath::Pow(-q, one_third);

			ValidRoots.Add(2 * u);
			ValidRoots.Add(-u);
		}
	}
	else if (d < 0) // three real solutions
	{
		// Fixing NaNs
		float cbp_sqrt = FMath::Sqrt(-cb_p);
		if (FMath::IsNearlyZero(cbp_sqrt))
			cbp_sqrt = 0.00001;

		const float phi = one_third * FMath::Acos(-q / cbp_sqrt);
		const float t = 2 * FMath::Sqrt(-p);
		const float pi_over_three = PI / 3.0f;

		ValidRoots.Add(t * FMath::Cos(phi));
		ValidRoots.Add(-t * FMath::Cos(phi + pi_over_three));
		ValidRoots.Add(-t * FMath::Cos(phi - pi_over_three));
	}
	else // one real solution
	{
		const float d_sqrt = FMath::Sqrt(d);
		const float u = FMath::Pow(d_sqrt - q, one_third);
		const float v = -FMath::Pow(d_sqrt + q, one_third);

		ValidRoots.Add(u + v);
	}

	// resub
	const float sub = one_third * a;

	for (float& currRoot : ValidRoots)
		currRoot -= sub;

	return ValidRoots.Num() > 0;
}

bool UHandyFuncs::SolveQuarticEquation(float A, float B, float C, float D, float E, TArray<float>& ValidRoots)
{
	// https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_ballistic_trajectory.cs#L146

	// Fixing NaNs
	if (FMath::IsNearlyZero(A))
		A = 0.00001;

	// shift to normal form (x^4 + ax^3 + bx^2 + cx + d)
	const float a = B / A;
	const float b = C / A;
	const float c = D / A;
	const float d = E / A;

	// sub x = y - a/4 to remove cubic term (x^4 + px^2 + qx + r)
	const float a_squared = a * a;
	const float p = -0.375f * a_squared + b;
	const float q = 0.125f * a_squared * a - 0.5f * a * b + c;
	const float r = -0.01171875f * a_squared * a_squared + 0.0625f * a_squared * b - 0.25f * a * c + d;

	if (FMath::IsNearlyZero(r)) // no absolute term: y(y^3 + py + q)
		SolveCubicEquation(1, 0, p, q, ValidRoots);
	else
	{
		// solve the resolvent cubic
		TArray<float> ResolventRoots;

		if (!SolveCubicEquation(1, -0.5f * p, -r, 0.5 * r * p - 0.125 * q * q, ResolventRoots))
			return false;

		// take the one real solution
		const float z = ResolventRoots[0];

		float u = z * z - r;
		float v = 2.0f * z - p;

		if (FMath::IsNearlyZero(u))
			u = 0;
		else if (u > 0)
			u = FMath::Sqrt(u);
		else
			return false;

		if (FMath::IsNearlyZero(v))
			v = 0;
		else if (v > 0)
			v = FMath::Sqrt(v);
		else
			return false;

		SolveQuadraticEquation(1, (q < 0 ? -v : v), z - u, ValidRoots);
		SolveQuadraticEquation(1, (q < 0 ? v : -v), z + u, ValidRoots);
	}

	const float sub = 0.25f * a;

	for (float& currRoot : ValidRoots)
		currRoot -= sub;

	return ValidRoots.Num() > 0;
}

bool UHandyFuncs::TimeToImpact_Gravity(FVector ShooterPosition, FVector TargetPosition, FVector TargetVelocity, FVector Gravity, float ProjectileSpeed, TArray<float>& FoundTimes)
{
	// oh my god I'm crying
	// I thought I'd be bashing my head against quartics forever
	// https://www.forrestthewoods.com/blog/solving_ballistic_trajectories/
	// I am saved
	// he has source for all the pieces and everything
	// Why can I only bookmark things once

	FVector ToTarg = TargetPosition - ShooterPosition;

	const float A = 0.25f * Gravity.SizeSquared();
	const float B = FVector::DotProduct(TargetVelocity, Gravity);
	const float C = FVector::DotProduct(ToTarg, Gravity) + TargetVelocity.SizeSquared() - (ProjectileSpeed * ProjectileSpeed);
	const float D = 2 * FVector::DotProduct(ToTarg, TargetVelocity);
	const float E = ToTarg.SizeSquared();

	if (A != 0)
		SolveQuarticEquation(A, B, C, D, E, FoundTimes);
	else if (B != 0)
		SolveCubicEquation(B, C, D, E, FoundTimes);
	else
		SolveQuadraticEquation(C, D, E, FoundTimes);

	FoundTimes.Sort();
	FoundTimes.RemoveAll([](float& a) { return a < 0; });
	return FoundTimes.Num() > 0;
}



FVector UHandyFuncs::KochanekBartelInterpolation(FVector P0, FVector P1, FVector P2, FVector P3, float Time, float Tension, float Bias, float Continuity)
{
	// http://paulbourke.net/miscellaneous/interpolation/
	// https://en.wikipedia.org/wiki/Kochanek%E2%80%93Bartels_spline

	const float RevTension = 1 - Tension;
	const float BiasAdd = 1 + Bias;
	const float BiasSub = 1 - Bias;
	const float ContAdd = 1 + Continuity;
	const float ContSub = 1 - Continuity;

	// Tangents to P1 and P2
	FVector T1 = ((P1 - P0) * ((RevTension * BiasAdd * ContAdd) / 2)) + ((P2 - P1) * ((RevTension * BiasSub * ContSub) / 2));
	FVector T2 = ((P2 - P1) * ((RevTension * BiasAdd * ContSub) / 2)) + ((P3 - P2) * ((RevTension * BiasSub * ContAdd) / 2));

	const float TimeSquared = Time * Time;
	const float TimeCubed = TimeSquared * Time;

	// Coefficients for hermite interpolation on 0-1 interval of time ( https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Unit_interval_(0,_1) )
	const float C0 = (2 * TimeCubed) - (3 * TimeSquared) + 1;
	const float C1 = TimeCubed - (2 * TimeSquared) + Time;
	const float C2 = (-2 * TimeCubed) + (3 * TimeSquared);
	const float C3 = TimeCubed - TimeSquared;

	return (C0 * P1) + (C1 * T1) + (C2 * P2) + (C3 * T2);
}


void UHandyFuncs::HideActorFromPlayerController(APlayerController* PlayerController, AActor* ActorToHide)
{
	if (nullptr != PlayerController && nullptr != ActorToHide)
		PlayerController->HiddenActors.AddUnique(ActorToHide);
}

void UHandyFuncs::HideComponentFromPlayerController(APlayerController* PlayerController, UPrimitiveComponent* CompToHide)
{
	if (nullptr != PlayerController && nullptr != CompToHide)
		PlayerController->HiddenPrimitiveComponents.AddUnique(CompToHide);
}

void UHandyFuncs::UnhideActorFromPlayerController(APlayerController* PlayerController, AActor* ActorToUnhide)
{
	if (nullptr != PlayerController && nullptr != ActorToUnhide)
		PlayerController->HiddenActors.Remove(ActorToUnhide);
}

void UHandyFuncs::UnhideComponentFromPlayerController(APlayerController* PlayerController, UPrimitiveComponent* CompToUnhide)
{
	if (nullptr != PlayerController && nullptr != CompToUnhide)
		PlayerController->HiddenPrimitiveComponents.Remove(CompToUnhide);
}


float UHandyFuncs::GetDistanceToMesh(UStaticMeshComponent* Comp, const FVector& Point, FVector& ClosestPoint)
{
	UStaticMesh* CompMesh = Comp->GetStaticMesh();
	if (nullptr != CompMesh && CompMesh->bAllowCPUAccess)
	{
		const FStaticMeshLODResources& LODZero = CompMesh->RenderData->LODResources[0];
		const FRawStaticIndexBuffer& Inds = LODZero.IndexBuffer;
		const FPositionVertexBuffer& Verts = LODZero.VertexBuffers.PositionVertexBuffer;

		float RollingDist = 9999999999.0f;
		bool FoundOne = false;
		for (int i = 0; i < Inds.GetNumIndices(); i += 3)
		{
			const FVector A = Comp->GetComponentTransform().TransformPosition(Verts.VertexPosition(Inds.GetIndex(i)));
			const FVector B = Comp->GetComponentTransform().TransformPosition(Verts.VertexPosition(Inds.GetIndex(i + 1)));
			const FVector C = Comp->GetComponentTransform().TransformPosition(Verts.VertexPosition(Inds.GetIndex(i + 2)));

			const FVector& TriClosest = FMath::ClosestPointOnTriangleToPoint(Point, A, B, C);
			const float TriClosestDist = FVector::Dist(TriClosest, Point);

			if (TriClosestDist < RollingDist)
			{
				RollingDist = TriClosestDist;
				ClosestPoint = TriClosest;
				FoundOne = true;
			}
		}

		if (FoundOne)
			return RollingDist;
	}

	ClosestPoint = Point;
	return -1.0f;
}

void UHandyFuncs::LawOfSinesAim2D(const FVector& ShooterPos,
	const FVector& FiringPos,
	const FVector& TargetPos,
	const FVector& TargetVelocity,
	const FVector& BulletVelocity,
	FVector& ResultAim,
	FVector& ExpectedIntersectPoint,
	float& ResultAngle)
{
	const FVector Up = FVector::UpVector;
	ResultAim = TargetPos - ShooterPos;

	const FVector TargVelNorm = TargetVelocity.GetSafeNormal();
	const FVector OldAimNorm = (ResultAim - (Up * (Up | ResultAim))).GetSafeNormal();
	const FVector OldAimRight = OldAimNorm.RotateAngleAxis(90, Up);
	const float TargSpeed = TargetVelocity.Size2D();
	const float BulletSpeed = BulletVelocity.Size2D();

	// Law of Sines. Works regardless of distance; all distances share this angle due to triangle similarity
	const float VelAng = FMath::Sin(PI - FMath::Acos(TargVelNorm | OldAimNorm));
	ResultAngle = FMath::Asin((TargSpeed * VelAng) / BulletSpeed);

	ResultAngle *= FMath::Sign(OldAimRight | TargVelNorm);

	ResultAim = ResultAim.RotateAngleAxis(FMath::RadiansToDegrees(ResultAngle), Up);
	FMath::SegmentIntersection2D(TargetPos, TargetPos + (TargVelNorm * 1000000), ShooterPos, ShooterPos + (ResultAim * 1000000), ExpectedIntersectPoint);

	if (!ShooterPos.Equals(FiringPos))
	{
		// Because of the offset, we actually have to do this twice.
		// First at the regular bullet speed, i.e. expecting no offset,
		// then at a higher bullet speed made to account for having an offset.

		const float ToTargSize = (ExpectedIntersectPoint - ShooterPos).Size2D();
		const float LengthFactor = (ToTargSize - (FiringPos - ShooterPos).Size2D()) / ToTargSize;

		if (LengthFactor <= 0)
		{
			// if the offset is larger than the entire length we travel, we can't really *do*
			// anything about that. Just fire directly at the target.

			ResultAim = TargetPos - ShooterPos;
		}
		else
		{
			ResultAngle = FMath::Asin((TargSpeed * VelAng) / (BulletSpeed / LengthFactor));

			ResultAngle *= FMath::Sign(OldAimRight | TargVelNorm);

			ResultAim = (TargetPos - ShooterPos).RotateAngleAxis(FMath::RadiansToDegrees(ResultAngle), Up);
			FMath::SegmentIntersection2D(ExpectedIntersectPoint, ExpectedIntersectPoint + (TargVelNorm * 1000000), ShooterPos, ShooterPos + (ResultAim * 1000000), ExpectedIntersectPoint);
		}
	}
}

void UHandyFuncs::GetLobArc_Simple(FVector ThrowPosition, FVector TargetPosition, float LobApexHeight, float LobTravelTime,
	FVector& OutThrowVelocity, float& OutThrowGravity)
{
	const FVector ToTarget = TargetPosition - ThrowPosition;

	/*
		In order to get both a consistent apex height *and* a consistent
		travel time, gravity has to change!

		Y1 = Y0 + Vy0*t - 0.5*G*t^2
		0 = Apex + 0 - 0.5*G*t^2
		Apex = 0.5*G*t^2

		Gravity = (2 * Apex) / t^2
	*/
	const float HalfThrowTime = LobTravelTime / 2; // Only half the time since the gravity calc is based on half the arc!
	OutThrowGravity = (2 * LobApexHeight) / (HalfThrowTime * HalfThrowTime);

	// Vertical velocity is naturally dictated by gravity and the apex
	const float VerticalVelocity = FMath::Sqrt(2 * OutThrowGravity * LobApexHeight);

	/*
		Lateral velocity, on the other hand, is directly correlated to time.

		Doesn't take into account elevation changes; this means it undershoots when
		lobbing upward and overshoots when lobbing downward!
	*/
	const float LateralVelocity = ToTarget.Size2D() / LobTravelTime;


	OutThrowVelocity = (ToTarget.GetSafeNormal2D() * LateralVelocity) + (FVector::UpVector * VerticalVelocity);

	OutThrowGravity *= -1;
}

void UHandyFuncs::GetLobArc_Complex(FVector ThrowPosition, FVector TargetPosition, float LobApexHeight, float LobTravelTime,
	FVector& OutThrowVelocity, float& OutThrowGravity)
{
	const FVector ToTarget = TargetPosition - ThrowPosition;

	// This implementation tries to take elevation into account
	// in order to land the lob at the target regardless of
	// how high or low it is.


	// one extremely psychotic wolfram alpha question later...
	// https://www.wolframalpha.com/input?i2d=true&i=solve+Sqrt%5B2*g*a%5D*t+-+Divide%5B1%2C2%5D*g*Power%5Bt%2C2%5D+%3D+h+for+g
	// (Solving "sqrt(2*g*a)*t - 0.5*g*t^2 = h for g", which is an expansion of the equation "v*t - 0.5g*t^2 = h")
	//
	// gravity in terms of apex (a), target height (h), and time taken to get there (t):
	//
	// g = (2*(+-2*sqrt(a^2*t^4 - aht^4) + 2*a*t^2 - h*t^2)) / t^4
	//
	// How did it get here? good question, but I'm not sure I want to know.

	const float TargetHeight = FMath::Min(ToTarget.Z, LobApexHeight);								// h
	const float ApexSq = LobApexHeight * LobApexHeight;											// a^2
	const float TimeSq = LobTravelTime * LobTravelTime;														// t^2
	const float TimeQu = TimeSq * TimeSq;															// t^4

	const float Eff1 = FMath::Sqrt((ApexSq * TimeQu) - (LobApexHeight * TargetHeight * TimeQu));	// sqrt(a^2*t^4 - aht^4)
	const float Eff2 = (2 * LobApexHeight * TimeSq);												// 2*a*t^2
	const float Eff3 = (TargetHeight * TimeSq);														// h*t^2

	const float Result1 = 2 * ((-2 * Eff1) + Eff2 - Eff3) / TimeQu;
	const float Result2 = 2 * (( 2 * Eff1) + Eff2 - Eff3) / TimeQu;

	//UKismetSystemLibrary::PrintString(this, "Complex Throw Res 1: " + FString::SanitizeFloat(Result1) +
	//	"\nComplex Throw Res 2: " + FString::SanitizeFloat(Result2));

	OutThrowGravity = FMath::Max(Result1, Result2);


	// And then the rest is normal

	const float VerticalVelocity = FMath::Sqrt(2 * OutThrowGravity * LobApexHeight);
	const float LateralVelocity = ToTarget.Size2D() / LobTravelTime;

	OutThrowVelocity = (ToTarget.GetSafeNormal2D() * LateralVelocity) + (FVector::UpVector * VerticalVelocity);

	OutThrowGravity *= -1;
}
