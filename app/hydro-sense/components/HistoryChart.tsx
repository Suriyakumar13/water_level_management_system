// components/HistoryChart.tsx
import React, { useEffect, useState } from 'react';
import { View, Text } from 'react-native';
import { supabase } from '../supabase';
import { VictoryChart, VictoryLine, VictoryAxis } from 'victory-native';

export default function HistoryChart({ deviceParams, focused }: any) {
  const [data, setData] = useState<Array<{ x: Date; y: number }>>([]);

  useEffect(() => {
    if (!focused) return;
    (async () => {
      const device_id = deviceParams?.device_id;
      const since = new Date(Date.now() - 24*60*60*1000).toISOString();
      let { data: rows } = await supabase
        .from('water_levels')
        .select('water_level_cm, timestamp')
        .eq('device_id', device_id)
        .gt('timestamp', since)
        .order('timestamp', { ascending: true });
      setData(rows.map(r => ({ x: new Date(r.timestamp), y: r.water_level_cm })));
    })();
  }, [focused]);

  if (!data.length) {
    return <Text style={{ textAlign: 'center', marginTop: 20 }}>No history yet.</Text>;
  }

  return (
    <VictoryChart>
      <VictoryAxis fixLabelOverlap />
      <VictoryAxis dependentAxis />
      <VictoryLine data={data} interpolation="monotoneX" />
    </VictoryChart>
  );
}
