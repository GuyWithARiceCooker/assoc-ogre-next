/**
 * =============================================================================
 * assoc-next — Ogre-Next 2.3 önálló minta (natív 2.x stack)
 * =============================================================================
 *
 * A 2.3 hivatalos ösvénye: `Hlms` (Unlit + PBS) regisztráció, `Item` a meshre, `CompositorManager2`
 * alap munkatér, irányfény, `Ogre::Timer` + képkocka, és a v1 `Overlay` csak azért,
 * mert a motorban 2.x alatt is ez a támogatott HUD/szöveg ÚT (a meshbe quadozott
 * szöveg a másik, nehezebb műfaj).
 *
 * Erőforrás: a Hlms mapok `ASSOC_OGRE_NEXT_MEDIA_ROOT` alól (Samples/Media);
 * a gömb mesh + `DebugFont` a telepített SDK `DebugPack.zip`-jéből (`General` csoport).
 * Futtatás: a `build/` mappából: `plugins.cfg`, `ogre-next.cfg`, log relatív a CWD-hoz.
 * =============================================================================
 */

#include "assoc_config.h"

#include "OgreArchiveManager.h"
#include "OgreCamera.h"
#include "OgreFileSystemLayer.h"
#include "OgreLight.h"
#include "OgreLogManager.h" // LML_* (OgreLog)
#include "OgreResourceGroupManager.h"
#include "OgreRoot.h"
#include "OgreSceneNode.h"
#include "OgreWindow.h"
#include "OgreWindowEventUtilities.h"

#include "OgreHlmsManager.h"
#include "Hlms/Pbs/OgreHlmsPbs.h"     // 2.x: a fejlécek Hlms/Pbs/, nem a flat OGRE/
#include "Hlms/Unlit/OgreHlmsUnlit.h"

#include "Compositor/OgreCompositorManager2.h"

#include "OgreOverlaySystem.h"
#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayManager.h"
#include "OgreTextAreaOverlayElement.h"
#include "OgreFontManager.h" // .fontdef ScriptLoader: regisztráció a General init előtt kell

/**
 * Beregisztrálja a két fő Hlms útvonalat: Unlit, majd PBS. A `mediaRootWithSlash` a
 * `Samples/Media` alja perjellel (Ogre fájlnév-összefűzés, dupla perjel elkerülése).
 * Előfeltétel: a `Root` singleton már létezik.
 */
static void registerAssocHlms( Ogre::String const& mediaRootWithSlash )
{
    using namespace Ogre;
    ArchiveManager& am = ArchiveManager::getSingleton();
    String mainPath;
    StringVector libPaths;
    {
        HlmsUnlit::getDefaultPaths( mainPath, libPaths );
        Archive* mainArch = am.load( mediaRootWithSlash + mainPath, "FileSystem", true );
        ArchiveVec libArchs;
        for( size_t i = 0; i < libPaths.size(); ++i )
        {
            libArchs.push_back( am.load( mediaRootWithSlash + libPaths[i], "FileSystem", true ) );
        }
        HlmsUnlit* u = OGRE_NEW HlmsUnlit( mainArch, &libArchs );
        Root::getSingleton().getHlmsManager()->registerHlms( u );
    }
    {
        HlmsPbs::getDefaultPaths( mainPath, libPaths );
        Archive* mainArch = am.load( mediaRootWithSlash + mainPath, "FileSystem", true );
        ArchiveVec libArchs;
        for( size_t i = 0; i < libPaths.size(); ++i )
        {
            libArchs.push_back( am.load( mediaRootWithSlash + libPaths[i], "FileSystem", true ) );
        }
        HlmsPbs* p = OGRE_NEW HlmsPbs( mainArch, &libArchs );
        Root::getSingleton().getHlmsManager()->registerHlms( p );
    }
}

/** Ablak bezárás: message pumpból kilép a fő ciklusból. */
class AssocQuitListener : public Ogre::WindowEventListener
{
    bool mQuit{ false };

public:
    void windowClosed( Ogre::Window* ) override
    {
        mQuit = true;
    }
    bool getQuit() const
    {
        return mQuit;
    }
};

/**
 * sárga szövegsor a v1 `Overlay` panelre; a `DebugFont` a DebugPack-ből, nem `.fontdef`
 * manuális fájlból, ha a zip már a `ResourceGroupManager`-nél (General).
 * `yNorm`: képernyőn normalizált függőleges (0..1).
 */
static Ogre::v1::TextAreaOverlayElement* addYellowLine( Ogre::v1::OverlayManager& om,
    Ogre::v1::OverlayContainer* panel, Ogre::String const& name, Ogre::String const& text,
    float yNorm )
{
    Ogre::v1::TextAreaOverlayElement* ta = static_cast<Ogre::v1::TextAreaOverlayElement*>(
        om.createOverlayElement( "TextArea", name ) );
    ta->setFontName( "DebugFont" );
    ta->setCharHeight( 0.03f );
    ta->setColour( Ogre::ColourValue( 1.0f, 0.88f, 0.15f, 1.0f ) );
    ta->setCaption( text );
    ta->setPosition( 0.04f, yNorm );
    panel->addChild( ta );
    return ta;
}

static Ogre::v1::TextAreaOverlayElement* addYellowText( Ogre::v1::OverlayManager& om,
    Ogre::v1::OverlayContainer* panel, Ogre::String const& name, Ogre::String const& text,
    float xNorm, float yNorm, float charHeight )
{
    Ogre::v1::TextAreaOverlayElement* ta = static_cast<Ogre::v1::TextAreaOverlayElement*>(
        om.createOverlayElement( "TextArea", name ) );
    ta->setFontName( "DebugFont" );
    ta->setCharHeight( charHeight );
    ta->setColour( Ogre::ColourValue( 1.0f, 0.88f, 0.15f, 1.0f ) );
    ta->setCaption( text );
    ta->setPosition( xNorm, yNorm );
    panel->addChild( ta );
    return ta;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, INT )
#else
int main( int, const char* [] )
#endif
{
    using namespace Ogre;
#ifndef OGRE_STATIC_LIB
#    if OGRE_DEBUG_MODE && !defined( __APPLE__ )
    const char* const plugFile = "plugins_d.cfg";
#    else
    const char* const plugFile = "plugins.cfg";
#    endif
#else
    const char* const plugFile = 0;
#endif
    // Munka könyvtár: build/ — a Root plugin listája, render config, log itt
    const String work = "./";
    Root* const root = OGRE_NEW Root( work + ( plugFile ? String( plugFile ) : String() ),
        work + "ogre-next.cfg", work + "Ogre-next.log" );

    // Első indításkor: Metal ha van; mármentett `ogre-next.cfg` esetén a `restoreConfig` megnyeri
    if( !root->restoreConfig() )
    {
        RenderSystem* chosen = 0;
        const RenderSystemList& lst = root->getAvailableRenderers();
        for( size_t i = 0; i < lst.size(); ++i )
        {
            if( lst[i]->getName().find( "Metal" ) != String::npos )
            {
                chosen = lst[i];
                break;
            }
        }
        if( !chosen && !lst.empty() )
        {
            chosen = lst[0];
        }
        if( !chosen )
        {
            OGRE_DELETE root;
            return -1;
        }
        root->setRenderSystem( chosen );
    }

    if( root->getRenderSystem() )
    {
        // macOS/Metal tipikus: sRGB, hogy a Hlms kimenet + ablak ne legyen túl sötét / hamis
        root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
    }

    Window* const window = root->initialise( true, "assoc  Ogre-Next" );

    // Hlms: sablon- és adat-útvonal; ha nincs a fa, a log kritikus, de a motor elindul (hibakereséshez)
    const String mediaRoot = String( ASSOC_OGRE_NEXT_MEDIA_ROOT ) + "/";
    if( !FileSystemLayer::fileExists( mediaRoot ) )
    {
        LogManager::getSingleton().logMessage(
            "Kritikus: nincs Hlms media: " + mediaRoot, LML_CRITICAL );
    }
    registerAssocHlms( mediaRoot );

    {
        const String pack = String( ASSOC_OGRE_NEXT_SDK ) + "/Media/packs/DebugPack.zip";
        ResourceGroupManager::getSingleton().addResourceLocation( pack, "Zip", "General", true );
    }
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups( false );

    SceneManager* const sm = root->createSceneManager( ST_GENERIC, 1u, "assoc_sm" );
    // v1: az `OverlaySystem` felhozza a `FontManager` singleton-t (a `libOgreOverlay` betöltése
    // önmagában nem; korai `FontManager::getSingleton()` ebben a futásban 0-ra süllyedhet).
    v1::OverlaySystem* const ovlSys = OGRE_NEW v1::OverlaySystem();
    sm->addRenderQueueListener( ovlSys );

    {
        // `DebugFont.fontdef`: a `General` init idején a Font ScriptLoader még nincs regisztrálva
        // (a logban csak a `.material` tűnik fel) — a zipből kézzel parse + betöltés, mielőtt
        // a `TextArea` a `setFontName`-t hívná.
        DataStreamPtr fontScript = ResourceGroupManager::getSingleton().openResource(
            "DebugPack/DebugFont.fontdef", "General", false );
        FontManager::getSingleton().parseScript( fontScript, "General" );
        FontPtr const dbg = FontManager::getSingleton().getByName( "DebugFont", "General" );
        if( dbg )
        {
            dbg->load();
        }
    }

    Camera* const camera = sm->createCamera( "main" );
    camera->setPosition( Vector3( 0, 1.0f, 5.0f ) );
    camera->lookAt( Vector3( 0, 0, 0 ) );
    camera->setNearClipDistance( 0.1f );
    camera->setFarClipDistance( 200.0f );
    camera->setAutoAspectRatio( true );

    // Egyetlen alap munkatér: háttérszín + a sm teljes 3D + overlay keverése
    CompositorManager2* const cm = root->getCompositorManager2();
    const String ws( "assoc_ws" );
    const ColourValue bg( 0.04f, 0.05f, 0.1f );
    cm->createBasicWorkspaceDef( ws, bg, IdString() );
    cm->addWorkspace( sm, window->getTexture(), camera, ws, true );

    // 2.x: `Light::setDirection` / `setPosition` csak *után* hívható, hogy a fény
    // egy `SceneNode`-hoz `attachObject` már megtörtént (különben `this` érvénytelen).
    Light* const l = sm->createLight();
    l->setType( Light::LT_DIRECTIONAL );
    l->setDiffuseColour( 1.0f, 0.95f, 0.85f );
    SceneNode* const lightNode = sm->getRootSceneNode()->createChildSceneNode();
    lightNode->attachObject( l );
    l->setDirection( Vector3( -0.2f, -0.6f, -0.4f ).normalisedCopy() );

    v1::OverlayManager& ovm = v1::OverlayManager::getSingleton();
    v1::Overlay* const o = ovm.create( "assoc_o" );
    v1::OverlayContainer* const pan =
        static_cast<v1::OverlayContainer*>( ovm.createOverlayElement( "Panel", "p" ) );
    addYellowText( ovm, pan, "assoc_center", "assoc", 0.40f, 0.42f, 0.12f );
    addYellowLine( ovm, pan, "t1", "assoc", 0.06f );
    addYellowLine( ovm, pan, "t2", "ASSOC  |  Ogre-Next  2.3", 0.10f );
    addYellowLine( ovm, pan, "t3", "a  s  s  o  c  —  Hlms + Item + sárga overlay (HUD v1)", 0.14f );
    o->add2D( pan );
    o->show();

    AssocQuitListener qu;
    WindowEventUtilities::addWindowEventListener( window, &qu );

    bool done = false;
    while( !done )
    {
        WindowEventUtilities::messagePump();
        done = qu.getQuit() || !root->renderOneFrame();
    }
    WindowEventUtilities::removeWindowEventListener( window, &qu );
    sm->removeRenderQueueListener( ovlSys );
    OGRE_DELETE ovlSys;
    OGRE_DELETE root;
    return 0;
}
