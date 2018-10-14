import Development.Shake
import Development.Shake.Command
import Development.Shake.FilePath
import Development.Shake.Util

import Data.List
import System.Directory
import Control.Monad
import System.IO
import Control.Monad.State

dropDir :: FilePath -> FilePath -> FilePath
dropDir x base = joinPath $ dropDir' (splitPath x) (splitPath base)
  where
    dropDir' :: [FilePath] -> [FilePath] -> [FilePath]
    dropDir' xs [] = xs
    dropDir' (x:xs) (b:bs) =
      let diff = x \\ b
      in if diff == "" || diff == "/" then
          dropDir' xs bs
        else
          error $ "could not drop '"++(joinPath (b:bs))++"' from '"++(joinPath (x:xs))++"'"


shakeOptions' :: ShakeOptions
shakeOptions' = shakeOptions{
  shakeFiles = "bld"</>"shake"</>"db",
  shakeReport = (("bld"</>"rep") </>) <$> [
    "t"<.>"trace", "h"<.>"html"
  ],
  shakeLint = Just LintBasic,
  shakeTimings = True
}

main :: IO ()
main = shakeArgs shakeOptions' $ do
  let path_in = "src"
  let path_out = "bld"
  let path_dep = path_out</>"dep"

  let path_objin  = path_in
  let path_objout = path_out</>"obj"
  let path_objdep = path_dep</>"obj"

  liftIO $ createDirectoryIfMissing True (path_out</>"rep")

  let exec = "dst"</>"dbg"</>"exec"
  want [exec]

  exec %> \out -> do
    let srcs = [
          path_objout</>"main"<.>"o"
          ]
    need srcs

    let libFlags = ("-l"++) <$> ["sdl2", "sly", "sdl2_image", "sdl2_ttf", "sdl2_mixer", "sdl2_gfx"]
    let libSearchFlags = ("-L"++) <$> ["/usr/local/opt/llvm/lib", "dep/lib"]

    () <- cmd "clang++" "-rpath" "@executable_path" "-O0" "-o" [out] libFlags libSearchFlags srcs

    libP <- liftIO $ makeAbsolute "dep/lib/libsly.dylib"
    () <- cmd "ln" "-sf" libP (takeDirectory out)
    return ()

  [path_objout<//>"*"<.>"o", path_objdep<//>"*"<.>"dep"] &%> \[out, dep] -> do
    let src = path_objin</>(dropDir out path_objout)-<.>"cpp"
    --  "-Ofast"
    let diagFlags = ["-fcolor-diagnostics"]
    let warnFlags = ["-Weverything", "-Wno-c++98-compat", "-Wno-c++98-c++11-compat", "-Wno-c++98-c++11-compat-pedantic", "-Wno-c99-extensions", "-Wno-c++98-c++11-c++14-compat", "-Wno-padded"]
    let outputFlags = ["-o", out]
    let includeFlags = ("-isystem"++) <$> ["/usr/local/include/SDL2", "/usr/local/opt/llvm/include", "dep/include/"]
    let otherFlags = ["-std=c++17"]
    let command = ["clang++", "-O0"] ++ diagFlags ++ warnFlags ++ outputFlags ++ includeFlags ++ otherFlags

    () <- cmd command "-M" "-MF" [dep] [src]
    needMakefileDependencies dep

    () <- cmd command "-c" [src]
    return ()

  phony "dbg" $ do
    return ()
