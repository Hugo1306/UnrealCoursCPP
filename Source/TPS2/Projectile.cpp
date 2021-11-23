// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h" 
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("StaticMesh'/Game/Geometry/Meshes/Sphere.Sphere'"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NomMesh"));

	if (SphereMesh.Succeeded())
		StaticMesh->SetStaticMesh(SphereMesh.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> SplashMat(TEXT("Material'/Game/ThirdPersonCPP/SplashMat.SplashMat'"));
	
	if(SplashMat.Succeeded())
		SplashMaterial = SplashMat.Object;


	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

	RootComponent = StaticMesh;

	SetActorScale3D(FVector(0.2f, 0.2f, 0.2f));

	SphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SphereCol->InitSphereRadius(50);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000;
	ProjectileMovement->ProjectileGravityScale = 0.5f;
	ProjectileMovement->Velocity = FVector(200, 0, 100);

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) 
{

	if (OtherActor->ActorHasTag(FName(TEXT("Wall"))))
	{

		FRotator RotationMat = FRotator(-90, Hit.Normal.Y, Hit.Normal.Z + 90);
		UGameplayStatics::SpawnDecalAtLocation(this, SplashMaterial, FVector(100, 100, 100), Hit.Location, RotationMat, 0);
		Destroy();

	}
	else
		Destroy();

}

