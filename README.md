# Wwise Gameplay Cues

An unreal engine plugin to support [Wwise](https://www.audiokinetic.com/en/wwise/overview/) Ak events inside GameplayCues

The Gameplay Ability System (GAS) comes with GameplayCues for vfx and sfx events.  
However, they only support engine sound classes and don't offer any compatibility with [Wwise](https://www.audiokinetic.com/en/wwise/overview/).
  
This plugin provides a bunch of FGameplayCueNotify_... struct wrappers, replacing the engine's gameplay cue ``USoundBase`` with an ``UAkAudioEvent`` class.

> [!NOTE]
> This plugin is in active development.
> Some things may not work as expected.

## Wrappers
The new ``FAkGameplayCueNotify_AkEventInfo`` replaces the old ``FGameplayCueNotify_SoundInfo``. Providing a UAkAudioEvent class to post when executing a GameplayCue.  
<img width="80%" height="80%" alt="image" src="https://github.com/user-attachments/assets/ffe1317a-9fea-4ebd-ba7d-f6ed406998de" />  

Following Gameplay Cue classes are included in this plugin to support Ak Audio Events:

### Gameplay Cue Classes
- [UAkGameplayCueNotify_Burst](/Source/WwiseGameplayCues/Public/AkGameplayCueNotify_Burst.h)
- [AAkGameplayCueNotify_BurstLatent](/Source/WwiseGameplayCues/Public/AkGameplayCueNotify_BurstLatent.h)
- [AAkGameplayCueNotify_Looping](/Source/WwiseGameplayCues/Public/AkGameplayCueNotify_Looping.h)


Additionally, the ``FAkGameplayCueNotify_AkEventInfo`` uses a ``FAkGameplayCueNotify_SpawnResult`` which is a copy of the ``FGameplayCueNotify_SpawnResult``, except that it stores a list of ``AkPlayingID`` Id's instead of audio components.  
@see [FAkGameplayCueNotify_AkEventInfo](/Source/WwiseGameplayCues/Public/AkGameplayCueTypes.h#L72)  
@see [FAkGameplayCueNotify_SpawnResult](/Source/WwiseGameplayCues/Public/AkGameplayCueTypes.h#L25)  
