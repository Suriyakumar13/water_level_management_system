// supabase.ts
import { createClient } from '@supabase/supabase-js';

const SUPABASE_URL    = 'https://gyznxothetefbdsxafcf.supabase.co';
const SUPABASE_ANON  = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Imd5em54b3RoZXRlZmJkc3hhZmNmIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDQxODQyNzIsImV4cCI6MjA1OTc2MDI3Mn0.uHc3SP7qRImJOreVYYrv5_GN4Di0_K5v_zdKkpgQR2w';

export const supabase = createClient(SUPABASE_URL, SUPABASE_ANON);
