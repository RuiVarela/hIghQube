plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val cmakeFile = "../../CMakeLists.txt"

fun computeVersionName() : String {
    val prefix = "set(VERSION_"
    val suffix = ")"

    var major = "0"
    var minor = "0"
    var build = "0"

    val file = project.file("../../CMakeLists.txt")

    if (file.exists()) {
        file.readLines()
            .filter { it.startsWith(prefix) && it.endsWith(suffix) }
            .map {
                val els = it
                    .removePrefix(prefix)
                    .removeSuffix(suffix)
                    .split(" ")
                els[0] to els[1]
            }
            .forEach { (name, value) ->
                when (name) {
                    "MAJOR" -> major = value
                    "MINOR" -> minor = value
                    "BUILD" -> build = value
                }
            }
    }
    return "$major.$minor.$build"
}

fun computeVersionCode() : Int {
    val version = computeVersionName()
    val els = version.split(".")
    return els[2].toInt()
}

android {
    namespace = "pt.demanda.highqube"
    compileSdk = 34

    defaultConfig {
        applicationId = "pt.demanda.highqube"
        minSdk = 27
        targetSdk = 34

        versionCode = computeVersionCode()
        versionName = computeVersionName()

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            // abiFilters.addAll(arrayOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
            // "armeabi-v7a" disabled due an issue on armv7+stb image resize

            abiFilters.addAll(arrayOf("arm64-v8a", "x86", "x86_64"))
        }
    }
    //logger.error("Version: ${defaultConfig.versionName} | ${defaultConfig.versionCode}")

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }

    externalNativeBuild {
        cmake {
            path = file(project.file("../../CMakeLists.txt"))
            version = "3.22.1"
        }
    }

    sourceSets {
        getByName("main") {
            assets {
                srcDirs("src/main/assets", "../../assets/bundled")
            }
        }
    }
}

dependencies {
    //implementation("androidx.core:core-ktx:1.12.0")
    //implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.10.0")
    //implementation("androidx.constraintlayout:constraintlayout:2.1.4")
}