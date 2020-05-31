// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SceneView.h"
#include "StaticFuncLib.generated.h"


/**
*
*/
UCLASS()
class HANDYFUNCS_API UHandyFuncs : public UObject
{
	GENERATED_BODY()

public:

	/*
		It may not look it, but this was two days of anguish.
		Was it worth it? Absolutely. Why would it be here if it wasn't? :^)

		Basically creates a quadratic bezier, i.e. a bezier with one control point,
		via a forward-differencing method similar to the one used by epic's cubic
		bezier function for efficiency. Basically, I use this whenever I need to
		make a visible parabolic trajectory in blueprints.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Quadratic Bezier Points"))
		static void MakeQBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, int NumPoints, TArray<FVector>& OutPoints);



	/*
		Various helpers for saving and loading files

		Handle these carefully! Spamming save/load dialogs
		seems to cause some issues with getting more to pop up.
	*/

	// C++ - only
	static TSharedPtr<FGenericWindow> GetMainNativeWindow();
	static TSharedPtr<FGenericWindow> GetNativeSubWindow(class UWidget* SubWindowContext);

	UFUNCTION(BlueprintCallable)
		static bool OpenFileDialog(
			const FString& DialogTitle,
			const FString& DefaultPath,
			const FString& DefaultFile,
			const FString& FileTypes,
			bool MultipleFiles,
			TArray<FString>& OutFilenames,
			class UWidget* SubWindowContext
		);

	UFUNCTION(BlueprintCallable)
		static bool SaveFileDialog(
			const FString& DialogTitle,
			const FString& DefaultPath,
			const FString& DefaultFile,
			const FString& FileTypes,
			bool MultipleFiles,
			TArray<FString>& OutFilenames,
			class UWidget* SubWindowContext
		);

	UFUNCTION(BlueprintCallable)
		static bool GetStringFromFile(FString& OutString, const FString& FilePath);

	UFUNCTION(BlueprintCallable)
		static bool SaveStringToFile(const FString& StringToSave, const FString& FilePath);



	/*
		An unhealthy amount of things to make color schemes because of my project's need for
		random color schemes for OCs.
		I get the feeling I'm going to want all of these sometime in the future.
	*/

	UFUNCTION(BlueprintPure)
		static void MakeComplementaryColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeAnalogousColorScheme(FLinearColor Root, int NumColors, float AngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeTriadicColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeSplitComplementaryColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeTetradicColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeSquareColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeMonochromeColorScheme(FLinearColor Root, int NumColors, FVector2D IntervalRange, TArray<FLinearColor>& OutColors);

	// and also a helper for making palette textures. basically just makes a line of pixels lol
	UFUNCTION(BlueprintCallable)
		static UTexture2D* MakeTextureForColorScheme(const TArray<FLinearColor>& Colors);



	/*
		This makes a texture out of a rendertarget at runtime. Incredibly handy for things like screenshots on save files!
	*/
	UFUNCTION(BlueprintCallable)
		static UTexture2D* MakeTextureFromRenderTarget(UTextureRenderTarget2D* Target, TArray<FLinearColor>& OutColors, bool InvertOpacity = true);



	/*
		This function is special!
		It creates a vector field which wraps around a mesh, which is super handy for some
		minor wrapping effects around meshes.

		Note that it's "special" in other ways too, i.e. it doesn't work in-editor, and it
		also has an issue with bounds sizes. Use with caution! If you live by the jank like
		I have, you may perish by it as I have as well. <-55,-55,-55> to <55,55,55> seems to work.

		As for using it:

		1) Find RenderToTexture_LevelBP in the Engine Content folder (you may have to unhide engine
		content), and drag one out into the scene.

		2) Set up the variables you need to set to put it into mesh-unwrapping mode.

		3) Set up two materials: one that displays local positions, and one that displays the vectors
		you want to make up the field. Remember to set 'allow negative emissive' and also set up
		the 'UV unwrap' material function so that it's wired to the world position offset output!

		4) Throw the mesh you want to be comformed to into the scene and wire it up to the
		RenderToTexture_LevelBP. Check 'unwrap' if you haven't already (or flick it off and on again if
		you have) to unwrap the mesh.

		5) If it doesn't show up in the preview view attached to the RenderToTexture_LevelBP, move it around
		a bit. There's some minor occlusion issues when working with it, so find a place where the mesh
		shows up in the view and stick with it.

		6) Render it out to a rendertarget of choice using the preview capture on the RenderToTexture_LevelBP
		(it's the capture that has a target on it), and then turn it into a texture from there. Do this with
		only the local position material on the mesh, and then again but only the vector texture.

		7) You're finally ready to use the function! Slot the local position and vector textures into the
		function, set the resolution, bounds, and path to save to, and the function will output an FGA
		file that you can drag into unreal. Technically, an FGA is just a frou-frou CSV, but saving it
		in this format lets unreal automatically recognize it as a vector field format.
	*/
	UFUNCTION(BlueprintCallable)
		static void ConstructVectorFieldFromUVs(FIntVector Res, FBox Bounds, UTexture2D* LocalPositions, UTexture2D* VectorValues, FString FilePath);



	/*
		Conversions between horizontal and vertical FOVs.
		Handy for cases when you want that as a graphics option for, say,
		ultra-widescreen support.

		I'm not quite sure how to get it to stay that way consistently yet,
		but at least having math when I do is handy.
	*/

	UFUNCTION(BlueprintPure)
		float ConvertHorizontalToVerticalFOV(float FOV, int ScreenWidth, int ScreenHeight);

	UFUNCTION(BlueprintPure)
		float ConvertVerticalToHorizontalFOV(float FOV, int ScreenWidth, int ScreenHeight);



	/*
		Projectile-leading solvers, i.e. funcs that find where you should aim in order to
		hit a target, moving or otherwise.

		Primarily comes from these sources:
		http://playtechs.blogspot.com/2007/04/aiming-at-moving-target.html
		http://inis.jinr.ru/sl/vol1/CMC/Graphics_Gems_1,ed_A.Glassner.pdf (Chapter VIII.1 - 'Cubic and Quartic Roots')
		https://www.forrestthewoods.com/blog/solving_ballistic_trajectories/
		https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_ballistic_trajectory.cs
	*/

	/*
		This function, plain and simple, finds the point to shoot at to hit a moving target with a
		zero-gravity bullet. It does one job, and it does it perfectly, and in a compact package.

		It returns a time value, though, so to get the point you'll want to do something like this:
		PointToAimAt = TargetPosition + (TargetVelocity * Time)
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Projectile Time to Impact (No Gravity)"))
		static float TimeToImpact_NoGravity(FVector ShooterPosition, FVector TargetPosition, FVector TargetVelocity, float ProjectileSpeed);

	/*
		Its big brother, on the other hand, takes a bit more to get working...
	*/

	// This solves quadratics, i.e. equations like Ax^2 + Bx + C, provided that you give it A, B, and C.
	UFUNCTION(BlueprintCallable)
		static bool SolveQuadraticEquation(float A, float B, float C, TArray<float>& ValidRoots);

	// This solves cubics, i.e. equations like Ax^3 + Bx^2 + Cx + D, provided that you give it A, B, C, and D.
	UFUNCTION(BlueprintCallable)
		static bool SolveCubicEquation(float A, float B, float C, float D, TArray<float>& ValidRoots);

	// This solves quartics, i.e. equations like Ax^4 + Bx^3 + Cx^2 + Dx + E, provided that you give it A, B, C, D, and E.
	UFUNCTION(BlueprintCallable)
		static bool SolveQuarticEquation(float A, float B, float C, float D, float E, TArray<float>& ValidRoots);

	/*
		This function is *incredible*! Slightly janky, but incredible.

		You can give it any direction of gravity, and it will work. You can give it no gravity, and it will behave just like
		the no-gravity case. It is, for all intents and purposes, a general ballistic solver.

		Just like its little brother, it returns times of impact. Not just one this time, though! This time, there's a low arc
		and a high arc, in that order, in the array it spits out. Choose whatever one suits your fancy.

		To find where you want to aim, you'll want to do something like this:
		PointToAimAt = TargetPosition + (TargetVelocity * Time) - (0.5 * Gravity * Time * Time)

		Note that this algorithm, while incredible, is definitely not perfect. Shot leading seems to be inherently
		tied to the gravity; Basically, if 0 < Gravity Size < 10, aiming breaks down for reasons I don't really
		understand. I would suggest using the No-Gravity function for negligible gravities instead, as it's slightly
		faster than doing it through this one.

		There also appear to be "blind spots" at certain points, where you can stand inside of them and the algorithm
		will produce no positive-time solutions even though there are definitely trajectories that would hit the player. I
		don't know enough about the math involved to solve this, so user beware!

		In reference to the source material: This is method one, the method described by James McNeill. I wrote it
		like this because I prefer method one's legibility and versatility over method two's. I have tested both;
		they both behave the same exact way, and have the same exact issues. One just happens to be more verbose
		than the other.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Projectile Time to Impact (Gravity)"))
		static bool TimeToImpact_Gravity(FVector ShooterPosition, FVector TargetPosition, FVector TargetVelocity, FVector Gravity, float ProjectileSpeed, TArray<float>& FoundTimes);



	/*
		Function for Kochanek-Bartels Spline Interpolation, from a long trek between a bunch of different sources:
		https://www.youtube.com/watch?v=LNidsMesxSE
		https://www.patreon.com/posts/overgrowth-like-27261934
		http://paulbourke.net/miscellaneous/interpolation/
		https://en.wikipedia.org/wiki/Kochanek%E2%80%93Bartels_spline
		https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Unit_interval_(0,_1)

		This function interpolates within the range of P1 to P2, using P0 and P3 to construct tangents,
		and additionally has tension, bias, and continuity variables for extra control.

		Parameter Cheat sheet:
		Tension: < 0 = Rounder, > 0 = Tighter
		Bias: < 0 = Bias toward P1, > 0 = Bias toward P2
		Continuity: < 0 = Loopier Corners, > 0 = Sharper Corners
	*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Kochanek-Bartels Spline Interpolation"))
		static FVector KochanekBartelInterpolation(FVector P0, FVector P1, FVector P2, FVector P3, float Time, float Tension, float Bias, float Continuity);



	// @
};
