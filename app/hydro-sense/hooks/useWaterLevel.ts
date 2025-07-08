// hooks/useWaterLevel.ts
import { useEffect, useState, useRef } from 'react';
import { supabase } from '../supabase';
import { getOrCreateDeviceId } from '../services/device';

export function useWaterLevel(
  dimensions: { length: number; width?: number },
  shape: 'cubical' | 'cylindrical'
) {
  const [level, setLevel]         = useState<number>(0);
  const [volume, setVolume]       = useState<number>(0);
  const [rate, setRate]           = useState<number>(0);
  const [leak, setLeak]           = useState<boolean>(false);
  const prevRef                   = useRef<{ lvl: number; ts: number }>();

  useEffect(() => {
    let subscription: any;
    (async () => {
      const device_id = await getOrCreateDeviceId();

      // initial fetch of latest level
      let { data } = await supabase
        .from('water_levels')
        .select('water_level_cm, timestamp')
        .eq('device_id', device_id)
        .order('timestamp', { ascending: false })
        .limit(1)
        .single();
      if (data) {
        updateAll(data.water_level_cm, new Date(data.timestamp).getTime());
      }

      // subscribe to real-time changes
      subscription = supabase
        .channel('water_updates')
        .on(
          'postgres_changes',
          { event: 'INSERT', schema: 'public', table: 'water_levels', filter: `device_id=eq.${device_id}` },
          payload => {
            const lvl = payload.new.water_level_cm as number;
            const ts  = new Date(payload.new.timestamp).getTime();
            updateAll(lvl, ts);
          }
        )
        .subscribe();
    })();

    function updateAll(currentLevel: number, ts: number) {
      setLevel(currentLevel);
      // volume
      if (shape === 'cubical') {
        setVolume(dimensions.length * (dimensions.width || dimensions.length) * currentLevel);
      } else {
        const r = dimensions.length / 2;
        setVolume(Math.PI * r * r * currentLevel);
      }
      // rate & leak
      if (prevRef.current) {
        const dt    = (ts - prevRef.current.ts) / (1000 * 60 * 60); // hours
        const dl    = prevRef.current.lvl - currentLevel;
        const rRate = dl / dt; 
        setRate(rRate);
        if (dl > 5 && dt < (10 / 60)) {
          setLeak(true);
        }
      }
      prevRef.current = { lvl: currentLevel, ts };
    }

    return () => supabase.removeChannel(subscription);
  }, [shape, dimensions]);

  return { level, volume, rate, leak };
}
